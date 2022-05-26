module;
#include <memory>
#include <Windows.h>
#include "CoreMacros.h"
module Allocator;
import Types;
import Helper;
import Global;

class BaseAllocator
{
public:
	// malloc 래핍
	static inline void* AllocateMemory(uint32 size)
	{
		return ::malloc(size);
	}

	static inline void	ReleaseMemory(void* ptr)
	{
		::free(ptr);
	}
};

class StompAllocator
{
public:
	enum { PAGE_SIZE = 0x1000 };

	static void* AllocateMemory(const uint32 size)
	{
		ASSERT(size == 0, "size must be larger than 0");

		const uint64 pageCount = (size + PAGE_SIZE - 1) / PAGE_SIZE;
		// const int64 pageCount = size > 0 ? (size / PAGE_SIZE) + 1 : 0;

		const uint64 offset = pageCount * PAGE_SIZE - size;

		void* baseAddress = ::VirtualAlloc(NULL, pageCount * PAGE_SIZE, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);

		void* ret = static_cast<uint8*>(baseAddress) + offset; // 사용할 메모리의 끝을 정확히 페이지(가상메모리 프레임) 끝에 맞추기
		return ret;
	}

	static void ReleaseMemory(const void* ptr)
	{
		const uint64 address = reinterpret_cast<uint64>(ptr);
		const uint64 baseAddress = address - (address % PAGE_SIZE);
		::VirtualFree(reinterpret_cast<void*>(baseAddress), 0, MEM_RELEASE);
	}
};

class PoolAllocator
{
public:
	static void* AllocateMemonry(uint32 _size)
	{
		return gMemoryManager->Allocate(_size);
	}

	static void ReleaseMemory(void* _ptr)
	{
		gMemoryManager->Release(_ptr);
	}
};
