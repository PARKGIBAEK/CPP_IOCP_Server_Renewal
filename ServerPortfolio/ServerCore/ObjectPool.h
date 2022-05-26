#pragma once
#include "Types.h"
#include "MemoryPool.h" 
//#include "Allocators.h"
/*
	※	MemoryPool과 ObjectPool의 차이

	 * Memory Pool :
	 - 미리 설정해둔 크기 별 Memory Pool을 공유하는 방식이다.
	   어떤 타입으로 메모리 사용했는지 정확히 알기 어렵다.
	   


	 * ObjectPool :
	 - static 맴버만 존재하는 template class이며,
	   호출 시 필요한 템플릿 인자 <Type>으로 부터 
	   Pooling할 객체 타입을 유추할 수 있다.

	   다시말해 각 타입마다 별개의 Memory Pool을 만들기 때문에
	   allocSize가 타입별로 다르다.
	   
	   메모리 할당/해제 시 오류가 발생할 경우
	   어떤 Type의 Object Pool에서 발생한 것인지 쉽게 파악할 수 있다.
*/

template<typename Type>
class ObjectPool
{
public:
	template<typename... Args>
	static Type* Pop(Args&&... _args)
	{
#if _STOMP_ALLOCATOR
		// StompAllocator는 메모리 할당 시 MemoryHeader사이를 위한 여유 메모리를 추가로 설정하지 않으니 주의!
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
		// 다시 반납
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