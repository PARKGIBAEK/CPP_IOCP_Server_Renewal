#include "Core.h"
#include "GlobalQueue.h"

GlobalQueue::GlobalQueue()
{
}

GlobalQueue::~GlobalQueue()
{
}

void GlobalQueue::Push(std::shared_ptr<JobQueue> _jobQueue)
{
	jobQueue.Push(_jobQueue);
}

std::shared_ptr<JobQueue> GlobalQueue::Pop()
{
	return jobQueue.Pop();
}
