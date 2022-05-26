#pragma once

class GlobalQueue
{
public:
	GlobalQueue();
	~GlobalQueue();
	
	void	Push(std::shared_ptr<JobQueue>_jobQueue);
	std::shared_ptr<JobQueue>	Pop();
private:
	LockQueue<std::shared_ptr<JobQueue>> jobQueue;
};

