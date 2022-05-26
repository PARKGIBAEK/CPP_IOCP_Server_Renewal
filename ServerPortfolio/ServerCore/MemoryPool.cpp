#include "Core.h"
#include "MemoryPool.h"

MemoryPool::MemoryPool(int32 _allocSize)
{
	::InitializeSListHead(&header);
}

MemoryPool::~MemoryPool()
{ // SLIST에서 꺼낸 메모리 해제
	while (MemoryHeader* memory = static_cast<MemoryHeader*>(::InterlockedPopEntrySList(&header)))
		::_aligned_free(memory);
}

void MemoryPool::Push(MemoryHeader* _ptr)
{
	_ptr->allocSize = 0;
	::InterlockedPushEntrySList(&header, _ptr);

	usedCount.fetch_sub(1);
	reservedCount.fetch_add(1);
}

MemoryHeader* MemoryPool::Pop()
{
	MemoryHeader* memory = static_cast<MemoryHeader*>(::InterlockedPopEntrySList(&header));
	if (memory == nullptr)
	{
		/* SLIST가 비어있어 아무것도 가져오지 못했으면 새로 할당하기
		_aligned_malloc은 CPU 아키텍쳐에 최적화된 동작을 위해 메모리 주소를 16의 배수로 맞춰준다
		( SLIST를 사용하려면 메모리 정렬을 16의 배수로 맞춰야 함)*/
		memory = reinterpret_cast<MemoryHeader*>(::_aligned_malloc(allocSize, SLIST_ALIGNMENT));
	}
	else
	{
		ASSERT_CRASH(memory->allocSize != 0,"allocSize is not 0");
		reservedCount.fetch_sub(1);
	}

	usedCount.fetch_add(1);

	return memory;
}