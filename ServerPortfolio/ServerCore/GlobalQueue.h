#pragma once

// �������� JobQueue�� ����
class GlobalQueue
{
public:
	GlobalQueue();
	~GlobalQueue();
	
	void	Push(std::shared_ptr<JobQueue>_jobQueue);
	// ó���ؾ� �� �۾��� �ִ� JobQueue�� ��ȯ
	std::shared_ptr<JobQueue>	Pop();

private:
	LockQueue<std::shared_ptr<JobQueue>> jobQueue;
};

