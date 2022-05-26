module;
#include <Windows.h>;
#include <atomic>;
#include "CoreMacros.h"
export module MemoryPool;
import Types;



enum { SLIST_ALIGNMENT = 16 };
// 메모리 할당 정보를 가지고 있는 헤더
export struct alignas(SLIST_ALIGNMENT) MemoryHeader : public SLIST_ENTRY
{
	MemoryHeader(uint32 _size) :allocSize(_size) {	}
	// BaseAddress에서 MemoryHeader만큼을 Offset으로 두고 다음 위치(실사용 위치)를 반환
	static void* AttachHeader(MemoryHeader* _header, uint32 _size)
	{
		new(_header) MemoryHeader(_size);

		return reinterpret_cast<void*>(++_header);
	}

	// BaseAddress위치를 반환
	static MemoryHeader* DetachHeader(void* _ptr)
	{
		MemoryHeader* header = reinterpret_cast<MemoryHeader*>(_ptr) - 1;
		return header;
	}

	uint32 allocSize;
};

// MS에서 제공하는 SLIST를 이용하여 Pooling
export class alignas(SLIST_ALIGNMENT) MemoryPool
{
public:
	MemoryPool(uint32 _allocSize) :allocSize(_allocSize)
	{
		::InitializeSListHead(&header);
	}

	~MemoryPool()
	{
		while (MemoryHeader* memory = static_cast<MemoryHeader*>(::InterlockedPopEntrySList(&header)))
			::_aligned_free(memory);
	}

	void Push(MemoryHeader* _ptr)
	{
		_ptr->allocSize = 0;
		::InterlockedPushEntrySList(&header, static_cast<PSLIST_ENTRY>(_ptr));

		usedCount.fetch_sub(1);
		reservedCount.fetch_add(1);
	}

	MemoryHeader* Pop()
	{
		MemoryHeader* memory = static_cast<MemoryHeader*>(::InterlockedPopEntrySList(&header));

		if (memory == nullptr)
		{// MS에서 제공하는 SLIST를 사용하려면 메모리 정렬을 반드시 16의 배수로 맞춰야 함(64비트 아키텍쳐에서 최적화된 동작을 위함)
			memory = reinterpret_cast<MemoryHeader*>(::_aligned_malloc(allocSize, SLIST_ALIGNMENT));
		}
		else
		{
			//Assert([=] {return memory->allocSize == 0; }, "allocSize must be larger than 0");
			ASSERT(memory->allocSize == 0, "allocSize must be larger than 0");
			reservedCount.fetch_sub(1);
		}

		usedCount.fetch_add(1);

		return memory;
	}

private:
	SLIST_HEADER			header; // 메모리 풀 역할
	uint32					allocSize = 0;
	std::atomic<uint32>		usedCount = 0; // 사용 중인 메모리
	std::atomic<uint32>		reservedCount = 0; // 메모리 풀의 여분
};