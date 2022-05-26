module;
#include <Windows.h>;
#include <atomic>;
#include "CoreMacros.h"
export module MemoryPool;
import Types;



enum { SLIST_ALIGNMENT = 16 };
// �޸� �Ҵ� ������ ������ �ִ� ���
export struct alignas(SLIST_ALIGNMENT) MemoryHeader : public SLIST_ENTRY
{
	MemoryHeader(uint32 _size) :allocSize(_size) {	}
	// BaseAddress���� MemoryHeader��ŭ�� Offset���� �ΰ� ���� ��ġ(�ǻ�� ��ġ)�� ��ȯ
	static void* AttachHeader(MemoryHeader* _header, uint32 _size)
	{
		new(_header) MemoryHeader(_size);

		return reinterpret_cast<void*>(++_header);
	}

	// BaseAddress��ġ�� ��ȯ
	static MemoryHeader* DetachHeader(void* _ptr)
	{
		MemoryHeader* header = reinterpret_cast<MemoryHeader*>(_ptr) - 1;
		return header;
	}

	uint32 allocSize;
};

// MS���� �����ϴ� SLIST�� �̿��Ͽ� Pooling
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
		{// MS���� �����ϴ� SLIST�� ����Ϸ��� �޸� ������ �ݵ�� 16�� ����� ����� ��(64��Ʈ ��Ű���Ŀ��� ����ȭ�� ������ ����)
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
	SLIST_HEADER			header; // �޸� Ǯ ����
	uint32					allocSize = 0;
	std::atomic<uint32>		usedCount = 0; // ��� ���� �޸�
	std::atomic<uint32>		reservedCount = 0; // �޸� Ǯ�� ����
};