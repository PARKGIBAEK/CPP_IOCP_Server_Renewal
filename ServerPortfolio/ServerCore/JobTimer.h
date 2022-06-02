#pragma once

struct JobData
{
	JobData(std::weak_ptr<class JobQueue> _owner, std::shared_ptr<Job> _job)
		:owner(_owner), job(_job)
	{

	}
	std::weak_ptr<JobQueue> owner; // Job�� ������ JobQueue
	std::shared_ptr<Job> job;
};

struct TimerItem
{
	bool operator<(const TimerItem& _other) const
	{
		return executeTick > _other.executeTick;
	}
	uint64 executeTick = 0;
	/* jobData�� shared_ptr�� ������ �ʰ�, raw pointer�� ���� ������ ?
	  - JobData�� �̸����� �Űܴٴϸ� Ref count �������� ���� ��� �߻� */
	JobData* jobData = nullptr;
};

class JobTimer
{
public:
	/* Priority_Queue�� Job�� executeTick�������� �ֱ�*/
	void		Reserve(uint64 _tickAfter,
		std::weak_ptr<JobQueue> _owner, std::shared_ptr<Job> _job);

	void		Reserve(TimerItem&& _timerItem);
	// TimerItem�� executeTick(���� �ð�)�� ���� Job���� owner(JobQueue)�� Push
	void		Distribute(uint64 _now);
	void		Clear();
private:
	USE_LOCK;
	PriorityQueue<TimerItem>	items;
	std::atomic<bool>			distributing = false;

};

