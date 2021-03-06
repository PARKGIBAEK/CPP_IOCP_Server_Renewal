#include "Core.h"
#include "JobQueue.h"
#include "GlobalQueue.h"
void JobQueue::Push(std::shared_ptr<Job> _job, bool _pushOnly)
{
	const int32 prevCount = jobCount.fetch_add(1);
	jobs.Push(_job); // WRITE_LOCK
	if (prevCount == 0)
	{
		if (tls_CurrentJobQueue == nullptr && _pushOnly == false)
			Execute();
		else /* GlobalQueue에 전달하여 분산 처리*/
			G_GlobalQueue->Push(shared_from_this());
	}
}

void JobQueue::Execute()
{
	tls_CurrentJobQueue = this;
	
	while (true)
	{
		Vector<std::shared_ptr<Job>> tempJobs;
		jobs.PopAll(tempJobs);// WRITE_LOCK

		const int32 tempJobCount = static_cast<int32>(tempJobs.size());
		for (int32 i = 0; i < tempJobCount; i++)
			tempJobs[i]->Execute();

		// 남은 일감이 0개라면 더 이상 처리할 Job이 없으므로 종료
		if (jobCount.fetch_sub(tempJobCount) == tempJobCount)
		{
			tls_CurrentJobQueue = nullptr;
			return;
		}

		const uint64 now = ::GetTickCount64();
		// 시간 초과 시 GlobalQueue에 일감 넘기기
		if (now >= tls_EndTickCount) 
		{
			tls_CurrentJobQueue = nullptr;
			G_GlobalQueue->Push(shared_from_this());
			//break;
			return;
		}
	}
}