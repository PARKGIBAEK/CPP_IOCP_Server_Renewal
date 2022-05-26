#pragma once
#include "Types.h"
#include "MemoryPool.h" 
//#include "Allocators.h"
/*
	��	MemoryPool�� ObjectPool�� ����

	 * Memory Pool :
	 - �̸� �����ص� ũ�� �� Memory Pool�� �����ϴ� ����̴�.
	   � Ÿ������ �޸� ����ߴ��� ��Ȯ�� �˱� ��ƴ�.
	   


	 * ObjectPool :
	 - static �ɹ��� �����ϴ� template class�̸�,
	   ȣ�� �� �ʿ��� ���ø� ���� <Type>���� ���� 
	   Pooling�� ��ü Ÿ���� ������ �� �ִ�.

	   �ٽø��� �� Ÿ�Ը��� ������ Memory Pool�� ����� ������
	   allocSize�� Ÿ�Ժ��� �ٸ���.
	   
	   �޸� �Ҵ�/���� �� ������ �߻��� ���
	   � Type�� Object Pool���� �߻��� ������ ���� �ľ��� �� �ִ�.
*/

template<typename Type>
class ObjectPool
{
public:
	template<typename... Args>
	static Type* Pop(Args&&... _args)
	{
#if _STOMP_ALLOCATOR
		// StompAllocator�� �޸� �Ҵ� �� MemoryHeader���̸� ���� ���� �޸𸮸� �߰��� �������� ������ ����!
		MemoryHeader* ptr = reinterpret_cast<MemoryHeader*>(
			StompAllocator::AllocateMemory(s_allocSize));
		Type* memory = static_cast<Type*>(
			MemoryHeader::AttachHeader(ptr, s_allocSize));
#else
		Type* memory = static_cast<Type*>(
			MemoryHeader::AttachHeader(s_memoryPool.Pop(), s_allocSize));
#endif // _STOMP_ALLOCATOR
		new(memory) Type(std::forward<Args>(_args)...);

		return memory;
	}

	static void Push(Type* _obj)
	{
		_obj->~Type();
#if _STOMP_ALLOCATOR
		StompAllocator::ReleaseMemory(MemoryHeader::DetachHeader(_obj));
#else
		// �ٽ� �ݳ�
		s_memoryPool.Push(MemoryHeader::DetachHeader(_obj));
#endif // _STOMP_ALLOCATOR

	}

	template<typename... Args>
	static std::shared_ptr<Type> MakeShared(Args&&... _args)
	{
		std::shared_ptr<Type> ptr = { Pop(std::forward<Args>(_args)...),Push };
		return ptr;
	}
private:
	static int32 s_allocSize;
	static MemoryPool s_memoryPool;
};

template<typename Type>
int32 ObjectPool<Type>::s_allocSize = sizeof(Type) + sizeof(MemoryHeader);

template<typename Type>
MemoryPool ObjectPool<Type>::s_memoryPool{ s_allocSize };