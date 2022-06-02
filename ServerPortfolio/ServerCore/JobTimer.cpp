#include "Core.h"
#include "JobTimer.h"
#include "JobQueue.h"

void JobTimer::Reserve(uint64 _tickAfter, std::weak_ptr<JobQueue> _owner, std::shared_ptr<Job> _job)
{
	const uint64 executeTick = ::GetTickCount64() + _tickAfter;
	JobData* jobData = ObjectPool<JobData>::Pop(_owner, _job);

	WRITE_LOCK;
	items.push(TimerItem{ executeTick,jobData });
}

void JobTimer::Reserve(TimerItem&& _timerItem)
{ 
	WRITE_LOCK;
	items.push(_timerItem);
}

void JobTimer::Distribute(uint64 _now)
{
	if (distributing.exchange(true) == true)
		return;

	Vector<TimerItem> tempItems;

	{
		WRITE_LOCK;
		while (items.empty() == false)
		{
			const TimerItem& timerItem = items.top();
			if (_now < timerItem.executeTick)
				break;

			tempItems.emplace_back(timerItem);
			items.pop();
		}
	}

	// TimerItem들을 owner JobQueue로 돌려보냄
	for (TimerItem& item : tempItems)
	{
		if (std::shared_ptr<JobQueue> ownerJobQueue = item.jobData->owner.lock())
			ownerJobQueue->Push(item.jobData->job, true); // owner JobQueue에 Job을 Push
		// jobData는 raw pointer로 생성하였기 때문에 직접 메모리 풀에 반환해야 함
		ObjectPool<JobData>::Push(item.jobData);
	}

	distributing.store(false);
}

void JobTimer::Clear()
{
	WRITE_LOCK;

	while(items.empty() == false) 
	{
		const TimerItem& timerItem = items.top();
		ObjectPool<JobData>::Push(timerItem.jobData);
		items.pop();
	}
}