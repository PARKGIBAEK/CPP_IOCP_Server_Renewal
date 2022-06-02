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
	/* jobData를 shared_ptr로 만들지 않고, raw pointer로 만든 이유는 ?
	  - JobData가 이리저리 옮겨다니면 Ref count 변경으로 인한 비용 발생 */
	JobData* jobData = nullptr;
};

class JobTimer
{
public:
	/* Priority_Queue에 Job을 executeTick기준으로 넣기*/
	void		Reserve(uint64 _tickAfter,
		std::weak_ptr<JobQueue> _owner, std::shared_ptr<Job> _job);

	void		Reserve(TimerItem&& _timerItem);
	// TimerItem의 executeTick(실행 시각)이 지난 Job들을 owner(JobQueue)에 Push
	void		Distribute(uint64 _now);
	void		Clear();
private:
	USE_LOCK;
	PriorityQueue<TimerItem>	items;
	std::atomic<bool>			distributing = false;

};

