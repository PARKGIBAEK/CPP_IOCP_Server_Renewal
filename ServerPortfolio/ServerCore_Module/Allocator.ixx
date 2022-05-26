export module Allocator;

import Types;
import Helper;
//import Global;


export class BaseAllocator;

export class StompAllocator;

export class PoolAllocator;

export template<typename T>
class STL_Allocator;


template<typename T>
class STL_Allocator
{
public:
	using value_type = T;

	constexpr STL_Allocator() noexcept {}
	constexpr STL_Allocator(const STL_Allocator&) noexcept = default;
	constexpr ~STL_Allocator() = default;
	inline STL_Allocator& operator=(const STL_Allocator&) = default;

	T* allocate(size_t _count)
	{
		const uint32 size = _count * sizeof(T);
		return static_cast<T*>(PoolAllocator::AllocateMemonry(size));
	}

	T* deallocate(T* _ptr, const size_t _count)
	{
		PoolAllocator::ReleaseMemory(_ptr);
	}

	template<typename U>
	bool operator==(const STL_Allocator<U>&) noexcept
	{
		return true;
	}

	template<typename U>
	bool operator!=(const STL_Allocator<U>&) noexcept
	{
		return false;
	}
};