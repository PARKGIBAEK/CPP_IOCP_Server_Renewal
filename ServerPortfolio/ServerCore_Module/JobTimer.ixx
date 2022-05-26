module;
#include <memory>

export module Module;

import Types;
import JobQueue;
import Job;

struct JobData
{
	JobData(std::weak_ptr<JobQueue> _owner, std::shared_ptr<Job> _job):owner(_owner), job(_job)
	{}

	std::weak_ptr<JobQueue> owner;
	std::shared_ptr<Job> job;
};
