#pragma once

// 전역으로 JobQueue를 관리
class GlobalQueue
{
public:
	GlobalQueue();
	~GlobalQueue();
	
	void	Push(std::shared_ptr<JobQueue>_jobQueue);
	// 처리해야 할 작업이 있는 JobQueue를 반환
	std::shared_ptr<JobQueue>	Pop();

private:
	LockQueue<std::shared_ptr<JobQueue>> jobQueue;
};

