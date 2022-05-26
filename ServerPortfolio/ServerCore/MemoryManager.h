#pragma once

class MemoryManager
{
	enum
	{
		POOL_COUNT=(1024/32)+(1024/128)+(2048/256),
		MAX_ALLOC_SIZE=0x1000,
	};
public:
	MemoryManager();
	~MemoryManager();
	
	void*	Allocate(uint32 _size);
	void	Release(void* _ptr);

private:
	std::vector<MemoryPool*> pools;
	MemoryPool* poolTable[MAX_ALLOC_SIZE + 1];
};

template<typename T, typename... Args>
T* XNew(Args... _args)
{
	T* memory = static_cast<T*>(PoolAllocator::AllocateMemory(sizeof(T)));
	new(memory) T(std::forward<Args>(_args)...);
	return memory;
}

template<typename T>
void XDelete(T* _obj)
{// 소멸자 호출 후 메모리 풀에 반납
	_obj->~T();
	PoolAllocator::ReleaseMemory(_obj);
}