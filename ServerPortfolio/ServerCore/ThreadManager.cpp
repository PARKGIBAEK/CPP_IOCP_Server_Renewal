#include "Core.h"
#include "ThreadManager.h"
#include "CoreTLS.h"
#include "CoreGlobal.h"
#include "GlobalQueue.h"
ThreadManager::ThreadManager()
{
	InitTLS();
}

ThreadManager::~ThreadManager()
{
	Join();
	
}

void ThreadManager::Launch(std::function<void(void)> _callback)
{
	std::lock_guard guard(mtxLock);

	threads.emplace_back(std::thread([=]() {
		InitTLS();
		_callback();
		DestroyTLS();
		}));
}

void ThreadManager::Join()
{
	for (auto& thread : threads) {
		if (thread.joinable())
			thread.join();
	}
	threads.clear();
}

void ThreadManager::InitTLS()
{
	static std::atomic<uint32> s_ThreadId = 1;
	tls_ThreadId = s_ThreadId.fetch_add(1);
}

void ThreadManager::DestroyTLS()
{
	//... release resources
}

void ThreadManager::DoGlobalQueueWork()
{
	while (true)
	{
		uint64 now = ::GetTickCount64();
		if (now > tls_EndTickCount)
			break;

		std::shared_ptr<JobQueue> jobQueue= G_GlobalQueue->Pop();
		if (jobQueue == nullptr)
			break;

		jobQueue->Execute();
		
	}
}

void ThreadManager::DistributeReservedJobs()
{
	const uint64 now = ::GetTickCount64();
	G_JobTimer->Distribute(now);
}
