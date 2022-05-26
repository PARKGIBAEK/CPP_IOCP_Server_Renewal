#include "Core.h"
#include "MemoryManager.h"

MemoryManager::MemoryManager()
{
	uint32 size = 0;
	uint32 tableIndex = 0;
	/* ����� �ٸ� �޸� Ǯ ������ ����
	  MemoryManager Pool���� ���� �����Ѵ�.

	 - ���� ���
	   size�� 32�� ������ MemoryPool�� 1~32 byte ���� ũ���� �޸𸮸� �Ҵ� ��û�� ���.
	   size�� 64�� ������ MemoryPool�� 33~64 byte ���� ũ���� �޸𸮸� �Ҵ� ��û�� ���.
	   size�� 96...	65~96 byte...
	   size�� 128..	97~128 byte...
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
	// �޸� ���ʿ� MemoryHeader�� ���̱� ���� �߰� ���� ����
	const uint32 allocSize = _size + sizeof(MemoryHeader);

#if _STOMP_ALLOCATOR
	header = reinterpret_cast<MemoryHeader*>(StompAllocator::AllocateMemory(allocSize));
#else
	if (allocSize > MAX_ALLOC_SIZE)
	{
		// �ʹ� ū �������� �޸𸮴� Ǯ������ �ʱ�
		header = reinterpret_cast<MemoryHeader*>(
			::_aligned_malloc(allocSize, SLIST_ALIGNMENT));
	}
	else
	{
		// MemoryPool���� ��������
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