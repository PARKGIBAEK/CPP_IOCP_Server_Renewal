module;
#include <vector>
#include <type_traits>
export module MemoryManager;

import Allocator;
import Types;
import MemoryPool;

export class MemoryManager
{
public:
	enum
	{
		POOL_COUNT = (1024 / 32) + (1024 / 128) + (2048 / 256),
		MAX_ALLOC_SIZE = 0x1000,
	};

	MemoryManager()
	{ // 사이즈 별로 MemoryManager Pool 생성
		uint32 size = 0;
		uint32 tableIndex = 0;

		for (size = 32; size <= 1024; size += 32)
		{
			MemoryPool* pool = new MemoryPool(size);
			pools.emplace_back(pool);

			while (tableIndex <= size)
			{
				poolTable[tableIndex] = pool;
				tableIndex++;
			}
		}
		for (; size <= 2048; size += 128)
		{
			MemoryPool* pool = new MemoryPool(size);
			pools.emplace_back(pool);

			while (tableIndex <= size)
			{
				poolTable[tableIndex] = pool;
				tableIndex++;
			}
		}
		for (; size <= MAX_ALLOC_SIZE; size += 256)
		{
			MemoryPool* pool = new MemoryPool(size);
			pools.emplace_back(pool);

			while (tableIndex <= size)
			{
				poolTable[tableIndex] = pool;
				tableIndex++;
			}
		}
	}

	void* Allocate(int32 _size)
	{
		MemoryHeader* header = nullptr;
		const int32 allocSize = _size + sizeof(MemoryHeader);
#ifdef _STOMP_ALLOCATOR
		hedaer = reinterpret_cast<MemoryHeader*>(StompAllocator::AllocateMemory(allocSize));
#else
		if (allocSize > MAX_ALLOC_SIZE)
			header = reinterpret_cast<MemoryHeader*>(::_aligned_malloc(allocSize, SLIST_ALIGNMENT));
		else
			header = poolTable[allocSize]->Pop();
#endif
		return MemoryHeader::AttachHeader(header, allocSize); // Memoryheader의 끝부분인 실제 사용 영역 반환
	}
	void Release(void* _ptr)
	{

	}
private:
	std::vector<MemoryPool*> pools;
	MemoryPool* poolTable[MAX_ALLOC_SIZE + 1];
};

template<typename Type, typename... Args>
Type* XNew(Args&&... args)
{
	Type* memory = static_cast<Type*>(PoolAllocator::AllocateMemonry(sizeof(Type)));
	new(memory) Type(std::forward<Args>(args)...);
	return memory;
}

template<typename Type>
void XDelete(Type* _obj)
{
	_obj->~Type();
	PoolAllocator::ReleaseMemory(_obj);
}

template<typename Type, typename... Args>
auto MakeShared(Args&&... args)
{
	return std::shared_ptr<Type>(XNew<Type>(std::forward<Args>(args)...), XDelete<Type>);
	//return std::make_shared<Type>(XNew<Type>(std::forward<Args>(args)...), XDelete<Type>);
}