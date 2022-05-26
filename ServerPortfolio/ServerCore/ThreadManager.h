#pragma once
#include <thread>
#include <functional>
#include <mutex>

class ThreadManager
{
public:
	ThreadManager();
	~ThreadManager();

	void Launch(std::function<void(void)> _callback);
	void Join();
	static void InitTLS();
	static void DestroyTLS();

	static void DoGlobalQueueWork();
	static void DistributeReservedJobs();
	
private:
	std::mutex	mtxLock;
	std::vector<std::thread>	threads;
	
};

