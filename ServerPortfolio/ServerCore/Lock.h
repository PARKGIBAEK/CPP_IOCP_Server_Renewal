#pragma once
#include "Types.h"

class Lock
{
	/*--------------------------------------------------------------
	  32비트 변수 lockFlag : [WWWWWWWW][WWWWWWWW][RRRRRRRR][RRRRRRRR]
	  W : Write Flag ( Exclusive Lock Owner ThreadId )
	  R : Read Flag ( Shared Lock Count)
	--------------------------------------------------------------*/

	enum :uint32
	{
		ACQUIRE_TIMEOUT_TICK = 10000,
		MAX_SPIN_COUNT = 5000,
		WRITE_THREAD_MASK = 0xFFFF'0000,
		READ_COUNT_MASK = 0x0000'FFFF,
		EMPTY_FLAG = 0x0000'0000,
	};
public:
	void WriteLock(const char* _name);
	void WriteUnlock(const char* _name);
	void ReadLock(const char* _name);
	void ReadUnlock(const char* _name);
private:
	std::atomic<uint32> lockFlag = EMPTY_FLAG;
	uint16 writeCount = 0;
};

/*===================
	ReadLockGuard
===================*/
class ReadLockGuard
{
public:
	ReadLockGuard(Lock& _lock, const char* _name)
		:lock(_lock), name(_name)
	{
		lock.ReadLock(name);
	}

	~ReadLockGuard()
	{
		lock.ReadUnlock(name);
	}
private:
	Lock& lock;
	const char* name;
};

/*===================
	WriteLockGuard
===================*/
class WriteLockGuard
{
public:
	WriteLockGuard(Lock& _lock, const char* _name)
		:lock(_lock), name(_name)
	{
		lock.ReadLock(name);
	}

	~WriteLockGuard()
	{
		lock.ReadUnlock(name);
	}
private:
	Lock& lock;
	const char* name;
};