#include "CoreGlobal.h"
#include "Core.h"
#include "JobTimer.h"
#include "GlobalQueue.h"
#include "ThreadManager.h"
#include "SocketHelper.h"
#include "SendBuffer.h"

MemoryManager* G_MemoryManager = nullptr;
GlobalQueue* G_GlobalQueue = nullptr;
JobTimer* G_JobTimer = nullptr;
ThreadManager* G_ThreadManager = nullptr;
SendBufferManager* G_SendBufferManager = nullptr;

class CoreGlobal
{
public:
	CoreGlobal()
	{
		G_MemoryManager = new MemoryManager();
		G_GlobalQueue = new GlobalQueue();
		G_JobTimer = new JobTimer();
		G_ThreadManager = new ThreadManager();
		G_SendBufferManager = new SendBufferManager();
		SocketHelper::Init();
	}
};