#pragma once
#include <memory>

class BaseAllocator
{
public:
	static void*	AllocateMemory(int32 _size);
	static void		ReleaseMemory(void* _ptr);
};

class StompAllocator
{
	enum { PAGE_SIZE = 0x1000 };
public:
	static void*	AllocateMemory(int32 _size);
	// _ptr의 base address를 찾아서 메모리 해제
	static void		ReleaseMemory(void* _ptr);
};

class PoolAllocator
{
public:
	static void*	AllocateMemory(uint32 _size);
	static void		ReleaseMemory(void* _ptr,uint32 _pageCount=1);
};

// STL Container에서 Pooling을 사용하기 위한 Allocator
template<typename Type>
class STL_Allocator
{
public:
	using value_type = Type;

	STL_Allocator() {}

	template<typename Other>
	STL_Allocator(const STL_Allocator<Other>&) {}

	Type* allocate(size_t _count) {
		const uint32 size = static_cast<int32>(_count * sizeof(Type));
		return static_cast<Type*>(PoolAllocator::AllocateMemory(size));
	}

	void deallocate(Type* _ptr, size_t _count)
	{
		PoolAllocator::ReleaseMemory(_ptr);
	}

	template<typename U>
	bool operator==(const STL_Allocator<U>&) { return true; }

	template<typename U>
	bool operator!=(const STL_Allocator<U>&) { return false; }
};