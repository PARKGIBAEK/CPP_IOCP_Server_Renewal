module;
#include <memory>
export module JobQueue;

import Types;
import Job;
import TLS;

export class JobQueue : public std::enable_shared_from_this<JobQueue>
{
public:
	//using CallbackType = std::function<void()>;
	void DoAsync(CallbackType&& _callback)
	{
		//.. Push 추가
	}

	template<typename T, typename Ret, typename... Args>
	void DoAsync(Ret(T::* _memFunc)(Args...), Args... _args)
	{
		std::shared_ptr<T>  owner = std::static_pointer_cast<T>(shared_from_this());
		//.. Push 추가
	}

	void DoTimer(uint64 _tickAfter, CallbackType&& _callback)
	{
		//std::shared_ptr<Job> job;

	}

	template<typename T, typename Ret, typename... Args>
	void DoTimer(uint64 _tickAfter, Ret(T::* _memFunc)(Args...), Args... args)
	{
		std::shared_ptr<T> owner = std::static_pointer_cast<T>(shared_from_this());
	}

	//void ClearJobs() { jobs.Clear(); }

	void Push(std::shared_ptr<Job> _job, bool _pushOnly = false) {
		const int32 prevCount = jobCount.fetch_add(1);
		//jobs.Push(_job);

		if (prevCount == 0)
		{
			
		}
	}

protected:
	//LockQueue<std::shared_ptr<Job>>			jobs;
	std::atomic<int32>							jobCount = 0;
};