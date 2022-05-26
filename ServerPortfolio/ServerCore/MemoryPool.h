#pragma once

enum { SLIST_ALIGNMENT = 16 };


struct alignas(SLIST_ALIGNMENT) MemoryHeader :public SLIST_ENTRY
{
	MemoryHeader(uint32 _size):allocSize(_size)
	{	}
	/* 할당된 메모리(_header)의 앞부분을 MemoryHeader로 채운뒤 MemoryHeader가 끝나는 지점을 반환한다
	 _size는 MemoryHeader의 크기를 포함한다는 사실에 주의
	*/
	static void* AttachHeader(MemoryHeader* _header, uint32 _size)
	{
		new(_header) MemoryHeader(_size);
		// 헤더 다음 위치부터 사용하게될 영역에 해당
		return reinterpret_cast<void*>(++_header);
	}

	static MemoryHeader* DetachHeader(void* _ptr)
	{
		MemoryHeader* header = reinterpret_cast<MemoryHeader*>(_ptr) - 1;
		return header;
	}
	uint32 allocSize;
	uint32 pageCount;
};

// SLIST를 이용한 Memory Pooling 컨테이너
class alignas(SLIST_ALIGNMENT) MemoryPool
{
public:
	MemoryPool(int32 _allocSize);
	~MemoryPool();

	void				Push(MemoryHeader* _ptr);
	// Memory Pool에서 여분의 메모리를 꺼내줌(여분이 없을 경우 직접 메모리 할당하여 전달)
	MemoryHeader*		Pop();

private:
	/*
	SLIST는 MS가 제공하는 Interlocked Singly-Linked List이다.
	SLIST_HEADER는 SLIST의 시작 노드이다.
	내부에서 사용되는 노드는 SLIST_ENTRY 이다.*/
	SLIST_HEADER		header; // 메모리 풀의 컨테이너로 사용된
	int32				allocSize = 0;
	std::atomic<int32>	usedCount = 0; // Memory Pool에서 꺼내준 메모리 갯수
	std::atomic<int32>	reservedCount = 0; // Memory Pool에서 당장 꺼내쓸 수 있는 여분 갯수
};