#include "Core.h"
#include "MemoryManager.h"

MemoryManager::MemoryManager()
{
	uint32 size = 0;
	uint32 tableIndex = 0;
	/* 사이즈가 다른 메모리 풀 여러개 생성
	  MemoryManager Pool들을 각각 생성한다.

	 - 예를 들면
	   size가 32로 생성된 MemoryPool은 1~32 byte 단위 크기의 메모리를 할당 요청을 담당.
	   size가 64로 생성된 MemoryPool은 33~64 byte 단위 크기의 메모리를 할당 요청을 담당.
	   size가 96...	65~96 byte...
	   size가 128..	97~128 byte...
	*/
	for (size = 32; size <= 1024; size += 32)
	{
		// 32, 64, 96, 128, 160, 192, 224, 256...1024
		MemoryPool* pool = new MemoryPool(size);
		pools.emplace_back(pool);

		while (tableIndex <= size)
			poolTable[tableIndex++] = pool;
	}

	for (; size <= 2048; size += 128)
	{
		// 1152, 1280, 1408, 1536, 1664, 1792, 1920, 2048
		MemoryPool* pool = new MemoryPool(size);
		pools.emplace_back(pool);

		while (tableIndex <= size)
			poolTable[tableIndex++] = pool;
	}

	for (; size <= MAX_ALLOC_SIZE; size += 256)
	{
		// 2304, 2560, 2816, 3072, 3328, 3584, 3840, 4096
		MemoryPool* pool = new MemoryPool(size);
		pools.emplace_back(pool);

		while (tableIndex <= size)
			poolTable[tableIndex++] = pool;
	}
}

MemoryManager::~MemoryManager()
{
	for (MemoryPool* pool : pools)
		delete pool;

	pools.clear();
}

void* MemoryManager::Allocate(uint32 _size)
{
	MemoryHeader* header = nullptr;
	// 메모리 앞쪽에 MemoryHeader를 붙이기 위해 추가 공간 마련
	const uint32 allocSize = _size + sizeof(MemoryHeader);

#if _STOMP_ALLOCATOR
	header = reinterpret_cast<MemoryHeader*>(StompAllocator::AllocateMemory(allocSize));
#else
	if (allocSize > MAX_ALLOC_SIZE)
	{
		// 너무 큰 사이즈의 메모리는 풀링하지 않기
		header = reinterpret_cast<MemoryHeader*>(
			::_aligned_malloc(allocSize, SLIST_ALIGNMENT));
	}
	else
	{
		// MemoryPool에서 꺼내오기
		header = poolTable[allocSize]->Pop();
	}
#endif // _STOMP_ALLOCATOR

	return MemoryHeader::AttachHeader(header, allocSize);
}

void MemoryManager::Release(void* _ptr)
{
	MemoryHeader* header = MemoryHeader::DetachHeader(_ptr);
	const uint32 allocSize = header->allocSize;
	ASSERT_CRASH(allocSize == 0, "Unknown error - allocSize is zero");

#if _STOMP_ALLOCATOR
	StompAllocator::ReleaseMemory(header);
#else
	if (allocSize > MAX_ALLOC_SIZE)
	{
		::_aligned_free(header);
	}
	else
	{
		poolTable[allocSize]->Push(header);
	}
#endif // _STOMP_ALLOCATOR

}