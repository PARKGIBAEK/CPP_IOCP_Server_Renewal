#include "Core.h"
#include "Lock.h"
#include "CoreTLS.h"

void Lock::WriteLock(const char* _name)
{
#if _DEBUG

#endif // _DEBUG

	const uint32 writeThreadId = (lockFlag.load() & WRITE_THREAD_MASK) >> 16;
	if (writeThreadId == tls_ThreadId)
	{ // Write Lock을 잡고있는 Thread가 현재 Thread인 경우(재귀로 Lock을 잡은 경우)
		writeCount++;
		return;
	}

	const uint64 timeTickStarted = ::GetTickCount64();
	const uint32 desired = ((tls_ThreadId << 16) & WRITE_THREAD_MASK);
	while (true)
	{
		for (uint32 spinCount = 0; spinCount < MAX_SPIN_COUNT; spinCount++) {
			uint32 expected = EMPTY_FLAG;
			if (lockFlag.compare_exchange_strong(expected, desired))
			{
				writeCount++;
				return;
			}
		}
		// 시간이 너무 오래걸리면 로직 수정이 필요 함

		if (::GetTickCount64() - timeTickStarted >= ACQUIRE_TIMEOUT_TICK)
			FORCED_CRASH("Timeout - WriteLock");
		std::this_thread::yield();
	}
}

void Lock::WriteUnlock(const char* _name)
{
#if _DEBUG

#endif // _DEBUG
	// Write Lock이 점유된 상태에서 Read Lock이 동시에 점유되어 있다면 로직 오류
	if ((lockFlag.load() & READ_COUNT_MASK) != 0)
		FORCED_CRASH("Invaild unlock order - WriteUnlock");


	if (--writeCount == 0)
		lockFlag.store(EMPTY_FLAG);


}

void Lock::ReadLock(const char* _name)
{
#if _DEBUG

#endif // _DEBUG

	const uint32 writeThreadId = (lockFlag.load() & WRITE_THREAD_MASK) >> 16;
	if (tls_ThreadId == writeThreadId)
	{ // Write Lock을 점유한 쓰레드는 Read Lock을 점유해도 괜찮음
		lockFlag.fetch_add(1); // Read Lock을 점유한 쓰레드 갯수 증가
		return;
	}

	const uint64 timeTickStarted = ::GetTickCount64();
	while (true)
	{
		for (uint32 spinCount = 0; spinCount < MAX_SPIN_COUNT; spinCount++)
		{
			uint32 expected = (lockFlag.load() & READ_COUNT_MASK);
			// Write Lock 점유 중일 때는 Read Lock 점유 불가
			if (lockFlag.compare_exchange_strong(expected, expected + 1))
				return;
		}

		if (::GetTickCount64() - timeTickStarted >= ACQUIRE_TIMEOUT_TICK)
			FORCED_CRASH("Timeout - ReadLock");

		std::this_thread::yield();
	}
}

void Lock::ReadUnlock(const char* _name)
{
#if _DEBUG

#endif // _DEBUG
	if ((lockFlag.fetch_sub(1) & READ_COUNT_MASK) == 0)
		FORCED_CRASH("Multiple unlock - ReadUnlock");
}
