#include "Core.h"
#include "Lock.h"
#include "CoreTLS.h"

void Lock::WriteLock(const char* _name)
{
#if _DEBUG

#endif // _DEBUG

	const uint32 writeThreadId = (lockFlag.load() & WRITE_THREAD_MASK) >> 16;
	if (writeThreadId == tls_ThreadId)
	{ // Write Lock�� ����ִ� Thread�� ���� Thread�� ���(��ͷ� Lock�� ���� ���)
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
		// �ð��� �ʹ� �����ɸ��� ���� ������ �ʿ� ��

		if (::GetTickCount64() - timeTickStarted >= ACQUIRE_TIMEOUT_TICK)
			FORCED_CRASH("Timeout - WriteLock");
		std::this_thread::yield();
	}
}

void Lock::WriteUnlock(const char* _name)
{
#if _DEBUG

#endif // _DEBUG
	// Write Lock�� ������ ���¿��� Read Lock�� ���ÿ� �����Ǿ� �ִٸ� ���� ����
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
	{ // Write Lock�� ������ ������� Read Lock�� �����ص� ������
		lockFlag.fetch_add(1); // Read Lock�� ������ ������ ���� ����
		return;
	}

	const uint64 timeTickStarted = ::GetTickCount64();
	while (true)
	{
		for (uint32 spinCount = 0; spinCount < MAX_SPIN_COUNT; spinCount++)
		{
			uint32 expected = (lockFlag.load() & READ_COUNT_MASK);
			// Write Lock ���� ���� ���� Read Lock ���� �Ұ�
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
