#pragma once

struct JobData
{
	JobData(std::weak_ptr<class JobQueue> _owner, std::shared_ptr<Job> _job)
		:owner(_owner), job(_job)
	{

	}
	std::weak_ptr<JobQueue> owner; // Job을 실행할 JobQueue
	std::shared_ptr<Job> job;
};

struct TimerItem
{
	bool operator<(const TimerItem& _other) const
	{
		return executeTick > _other.executeTick;
	}
	uint64 executeTick = 0;
	/* shared_ptr을 사용하는 이유는 JobData가 이리저리 옮겨다니기 때문에
	Ref count 변경으로 인한 비용 낭비를 막기 위함 */
	JobData* jobData = nullptr;
};

class JobTimer
{
public:
	void		Reserve(uint64 _tickAfter,
		std::weak_ptr<JobQueue> _owner, std::shared_ptr<Job> _job);

	void		Reserve(TimerItem&& _timerItem);
	void		Distribute(uint64 _now);
	void		Clear();
private:
	USE_LOCK;
	PriorityQueue<TimerItem>	items;
	std::atomic<bool>			distributing = false;

};

