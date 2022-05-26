module;
#include <Windows.h>
#include <atomic>
#include <thread>
#include "CoreMacros.h"
export module Lock;

import Types;
import TLS;
import Global;

export class Lock
{
public:
	enum : uint32
	{
		ACQUIRE_TIMEOUT_TICK = 10000,
		MAX_SPIN_COUNT = 5000,
		WRITE_THREAD_MASK = 0xFFFF'0000,
		READ_COUNT_MASK = 0x0000'FFFF,
		EMPTY_FLAG = 0x0000'0000,
	};

	void WriteLock(const char* _name) {
#if _DEBUG
		// Dead-Lock Profiler
		gDeadlockProfiler->PushNode(_name);
#endif
		const uint32 lockThreadId = (lockFlag.load() & WRITE_THREAD_MASK) >> 16;
		if (tls_ThreadId == lockThreadId)
		{
			writeCount++;
			return;
		}

		const uint64 beginTick = ::GetTickCount64();
		const uint32 desired = ((tls_ThreadId << 16) & WRITE_THREAD_MASK);
		while (true)
		{
			for (uint32 spinCount = 0; spinCount < MAX_SPIN_COUNT; spinCount++)
			{
				uint32 expected = EMPTY_FLAG;
				if (lockFlag.compare_exchange_strong(OUT expected, desired))
				{
					writeCount++;
					return;
				}
			}

			//assert(::GetTickCount64() - beginTick < ACQUIRE_TIMEOUT_TICK && "ACQUIRE_TIMEOUT_TICK");
			//Assert([=] {return ::GetTickCount64() - beginTick < ACQUIRE_TIMEOUT_TICK; }, "ACQUIRE_TIMEOUT_TICK");
			ASSERT(::GetTickCount64() - beginTick < ACQUIRE_TIMEOUT_TICK, "ACQUIRE_TIMEOUT_TICK");
			std::this_thread::yield();
		}

	}

	void WriteUnlock(const char* _name)
	{
#ifdef _DEBUG
		gDeadlockProfiler->PopNode(_name);
#endif

		//Assert([=] {return lockFlag.load() & READ_COUNT_MASK != 0; }, "Abnormal lock procedure detected");
		ASSERT((lockFlag.load() & READ_COUNT_MASK) != 0, "Abnormal lock procedure detected");
		const int32 lockCount = --writeCount;
		if (lockCount == 0)
			lockFlag.store(EMPTY_FLAG);
	}

	void ReadLock(const char* _name)
	{
#ifdef _DEBUG
		gDeadlockProfiler->PushNode(_name);
#endif
		const uint32 lockThreadId = (lockFlag.load() & WRITE_THREAD_MASK) >> 16;
		if (tls_ThreadId == lockThreadId)
		{
			lockFlag.fetch_add(1); // READ_COUNT_MASK 증가
			return;
		}

		const int64 beginTick = ::GetTickCount64();
		while (true)
		{
			for (uint32 spinCount = 0; spinCount < MAX_SPIN_COUNT; spinCount++)
			{
				uint32 expected = (lockFlag.load() & READ_COUNT_MASK);
				if (lockFlag.compare_exchange_strong(OUT expected, expected + 1))
					return;
			}
			//Assert([=] {return ::GetTickCount64() - beginTick < ACQUIRE_TIMEOUT_TICK; }, "Time out - ReadLock");
			ASSERT(::GetTickCount64() - beginTick < ACQUIRE_TIMEOUT_TICK, "Time out - ReadLock");

			std::this_thread::yield();
		}
	}

	void ReadUnlock(const char* _name)
	{
#if _DEBUG
		gDeadlockProfiler->PopNode(_name);
#endif
		// READ_COUNT_MASK가 0보다 작아지면 뭔가 잘못된 것
		//Assert([=] {return(lockFlag.fetch_sub(1) & READ_COUNT_MASK) == 0; }, "READ_COUNT_MASK is less than 0");
		ASSERT((lockFlag.fetch_sub(1) & READ_COUNT_MASK) == 0, "READ_COUNT_MASK is less than 0");




	}
private:
	std::atomic<uint32> lockFlag = EMPTY_FLAG;// 상위 16비트 : Write_THREAD_MASK, 하위 16비트 : READ_COUNT_MASK
	uint16  writeCount = 0;
};

export class ReadLockGuard
{
public:
	ReadLockGuard(Lock& _lock, const char* _name) :lock(_lock), name(_name)
	{
		lock.ReadLock(_name);
	}

	~ReadLockGuard()
	{
		lock.ReadUnlock(name);
	}
private:
	Lock& lock;
	const char* name;
};

export class WriteLockGuard
{
public:
	WriteLockGuard(Lock& _lock, const char* _name) : lock(_lock), name(_name)
	{
		lock.WriteLock(_name);
	}
	~WriteLockGuard()
	{
		lock.WriteUnlock(name);
	}
private:
	Lock& lock;
	const char* name;
};