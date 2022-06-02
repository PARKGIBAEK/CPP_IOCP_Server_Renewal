#pragma once

enum { SLIST_ALIGNMENT = 16 };


struct alignas(SLIST_ALIGNMENT) MemoryHeader :public SLIST_ENTRY
{
	MemoryHeader(uint32 _size):allocSize(_size)
	{	}

	/* �Ҵ�� �޸�(_header)�� �պκп� MemoryHeader ������ ȣ�� ��
		MemoryHeader������ ��ŭ �ڷ� �ѱ� �޸� �ּҸ� ��ȯ.
	  �� _size�� MemoryHeader�� ũ�⸦ �����Ѵٴ� ��ǿ� ����
	*/
	static void* AttachHeader(MemoryHeader* _header, uint32 _size)
	{
		new(_header) MemoryHeader(_size);
		// ��� ���� ��ġ���� ����ϰԵ� ������ �ش�
		return reinterpret_cast<void*>(++_header);
	}

	// �޸� �ּҸ� MemoryHeader������ ��ŭ �մ�� ��ġ ��ȯ
	static MemoryHeader* DetachHeader(void* _ptr)
	{
		MemoryHeader* header = reinterpret_cast<MemoryHeader*>(_ptr) - 1;
		return header;
	}
	uint32 allocSize;
};

// SLIST�� �̿��� Memory Pooling �����̳�
class alignas(SLIST_ALIGNMENT) MemoryPool
{
public:
	MemoryPool(int32 _allocSize);
	~MemoryPool();

	// �� �� �޸� SLIST�� ����
	void				Push(MemoryHeader* _ptr);
	// SLIST���� ������ �޸𸮸� ������ ��ȯ(������ ���� ��� �Ҵ��Ͽ� ��ȯ)
	MemoryHeader*		Pop();

private:
	/*
	SLIST�� MS�� �����ϴ� Interlocked Singly-Linked List�̴�.
	SLIST_HEADER�� SLIST�� ���� ����̴�.
	���ο��� ���Ǵ� ���� SLIST_ENTRY �̴�.*/
	SLIST_HEADER		header; // �޸� Ǯ�� �����̳ʷ� ����
	int32				allocSize = 0;
	std::atomic<int32>	usedCount = 0; // Memory Pool���� ������ �޸� ����
	std::atomic<int32>	reservedCount = 0; // Memory Pool���� ���� ������ �� �ִ� ���� ����
};