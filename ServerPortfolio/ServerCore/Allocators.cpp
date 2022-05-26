#include "Core.h"
#include "Allocators.h"

void* BaseAllocator::AllocateMemory(int32 _size)
{
	//return ::malloc(_size);
	return nullptr;
}

void BaseAllocator::ReleaseMemory(void* _ptr)
{
	::free(_ptr);
}

void* StompAllocator::AllocateMemory(int32 _size)
{
	/* ���ǻ���
	- StompAllocator���� PAGE_SIZE ũ���� ������ ũ���� �޸𸮸� �Ҵ���� ��
	  MemoryHeader����� ���� ���� ������ ������� �ʱ� ������,
	  PAGE_SIZEũ���� �־��� �޸� �� ������ MemoryHeader�� ����Ѵ�.
	  ���� _size + sizeof(MemoryHeader) <= PAGE_SIZE �� ���Ѿ��Ѵ� !!
	*/
	ASSERT_CRASH(_size == 0 || _size > PAGE_SIZE, "_size is zero OR exceeded PAGE_SIZE");
	const int64 pageCount = (_size + PAGE_SIZE - 1) / PAGE_SIZE;

	const uint64 offset = pageCount * PAGE_SIZE - _size;
	// MEM_RESERVE | MEM_COMMIT : ����� ���ÿ� �Ҵ�
	// PAGE_READWRITE : �б�/���� ����
	void* baseAddress = ::VirtualAlloc(NULL, pageCount * PAGE_SIZE, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
	/* �Ҵ�� �޸� ������ ��� ��� Access Violation�� �߻��� �� �ֵ���
	   ��û ������ ��ŭ�� �޸𸮸� ������ ���� �� ���缭 ��ȯ*/
	void* ret = static_cast<uint8*>(baseAddress) + offset;
	return ret;
}

void StompAllocator::ReleaseMemory(void* _ptr)
{
	const uint64 address = reinterpret_cast<uint64>(_ptr);
	/* �� ���ǻ���
	- AllocateMemory �Լ����� pageCount�� 1�� �ʰ��� ��� baseAddress ��꿡 �����ؾ��Ѵ�.
	  ���� ASSERT_CRASH�κ��� _size�� ������ ������ �ǵ帮�� ����.
	  �ؾ��� ���� ��������.
	- MemoryPool�� ����� ��� �޸� ���� method�� �̸� ����� �����ؼ� ���.
	*/
	const uint64 baseAddress = address - (address % PAGE_SIZE);

	::VirtualFree(reinterpret_cast<void*>(baseAddress), 0, MEM_RELEASE);
}

void* PoolAllocator::AllocateMemory(uint32 _size)
{
	return G_MemoryManager->Allocate(_size);
}

void PoolAllocator::ReleaseMemory(void* _ptr, uint32 _pageCount)
{
	G_MemoryManager->Release(_ptr);
}
