module;
#include <thread>
#include <functional>
#include <mutex>
#include <vector>
#include <Windows.h>
export module ThreadManager;

import Types;
import TLS;
import JobQueue;

export class ThreadManager
{
public:
	ThreadManager()
	{
		InitTLS();
	}

	~ThreadManager()
	{
		Join();
	}

	void Launch(std::function<void()> _callback)
	{
		std::lock_guard guard(mtx);

		threads.push_back(
			std::thread([=]()
			{
				InitTLS();
				_callback();
				DestroyTLS();
			})
		);
	}

	void Join()
	{
		for (auto& thread : threads)
		{
			if (thread.joinable())
				thread.join();
			threads.pop_back();
		}
	}

	static void InitTLS() 
	{
		// 쓰레드별 번호 부여
		static std::atomic<uint32> sThreadId = 1;
		tls_ThreadId = sThreadId.fetch_add(1);
	}

	static void DestroyTLS() 
	{
		// ...
	}

	static void DoGlobalQueueWork()
	{
		while (true)
		{
			uint64 now = ::GetTickCount64();
			if (now > tls_EndTickCount)
				break;

			/*std::shared_ptr<JobQueue> jobQueue = global_Queue->Pop();
			if (jobQueue == nullptr)
				break;

			jobQueue->Excute();*/
		}
	}

	static void DistributeReservedJobs()
	{
		const uint64 now = ::GetTickCount64();

		//global_JobTimer->Distribute(now);
	}
private:
	std::mutex mtx;
	std::vector<std::thread> threads;
};

