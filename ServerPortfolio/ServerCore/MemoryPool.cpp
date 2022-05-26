#include "Core.h"
#include "MemoryPool.h"

MemoryPool::MemoryPool(int32 _allocSize)
{
	::InitializeSListHead(&header);
}

MemoryPool::~MemoryPool()
{ // SLIST���� ���� �޸� ����
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
		/* SLIST�� ����־� �ƹ��͵� �������� �������� ���� �Ҵ��ϱ�
		_aligned_malloc�� CPU ��Ű���Ŀ� ����ȭ�� ������ ���� �޸� �ּҸ� 16�� ����� �����ش�
		( SLIST�� ����Ϸ��� �޸� ������ 16�� ����� ����� ��)*/
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