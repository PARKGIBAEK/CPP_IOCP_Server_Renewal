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
	/* 주의사항
	- StompAllocator에서 PAGE_SIZE 크기의 고정된 크기의 메모리를 할당받을 때
	  MemoryHeader사이즈를 위한 여유 공간을 고려하지 않기 때문에,
	  PAGE_SIZE크기의 주어진 메모리 중 앞쪽을 MemoryHeader로 사용한다.
	  따라서 _size + sizeof(MemoryHeader) <= PAGE_SIZE 를 지켜야한다 !!
	*/
	ASSERT_CRASH(_size == 0 || _size > PAGE_SIZE, "_size is zero OR exceeded PAGE_SIZE");
	const int64 pageCount = (_size + PAGE_SIZE - 1) / PAGE_SIZE;

	const uint64 offset = pageCount * PAGE_SIZE - _size;
	// MEM_RESERVE | MEM_COMMIT : 예약과 동시에 할당
	// PAGE_READWRITE : 읽기/쓰기 가능
	void* baseAddress = ::VirtualAlloc(NULL, pageCount * PAGE_SIZE, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
	/* 할당된 메모리 영역을 벗어날 경우 Access Violation이 발생할 수 있도록
	   요청 사이즈 만큼의 메모리를 프레임 끝에 딱 맞춰서 반환*/
	void* ret = static_cast<uint8*>(baseAddress) + offset;
	return ret;
}

void StompAllocator::ReleaseMemory(void* _ptr)
{
	const uint64 address = reinterpret_cast<uint64>(_ptr);
	/* ※ 주의사항
	- AllocateMemory 함수에서 pageCount가 1을 초과할 경우 baseAddress 계산에 주의해야한다.
	  따라서 ASSERT_CRASH부분의 _size의 제한은 가급적 건드리지 말자.
	  해야할 것이 많아진다.
	- MemoryPool을 사용할 경우 메모리 해제 method에 미리 사이즈를 지정해서 사용.
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
