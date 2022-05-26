#pragma once
#include "Job.h"
#include "LockQueue.h"
#include "JobTimer.h"

class JobQueue :public std::enable_shared_from_this<JobQueue>
{
public:
	void DoAsync(CallbackType&& _callback)
	{
		Push(ObjectPool<Job>::MakeShared(std::move(_callback)));
	}

	template<typename Type, typename RetType, typename... Args>
	void DoAsync(RetType(Type::* _memFunc)(Args...), Args... _args)
	{
		std::shared_ptr<Type> owner = static_pointer_cast<Type>(shared_from_this());
		Push(ObjectPool<Job>::MakeShared(owner, _memFunc, std::forward<Args>(_args)...));
	}

	void DoTimer(uint64 _tickAfter, CallbackType&& _callback)
	{
		std::shared_ptr<Job> job = ObjectPool<Job>::MakeShared(std::move(_callback));
		G_JobTimer->Reserve(_tickAfter, shared_from_this(), job);
	}

	template<typename Type, typename RetType, typename... Args>
	void DoTimer(uint64 _tickAfter, RetType(Type::* _memFunc)(Args ...), Args... _args)
	{
		std::shared_ptr<Type> owner = static_pointer_cast<Type>(shared_from_this());
		std::shared_ptr<Job> job = ObjectPool<Job>::MakeShared(owner, _memFunc, std::forward<Args>(_args)...);
		G_JobTimer->Reserve(_tickAfter, shared_from_this(), job);
	}

	void ClearJob() { jobs.Clear(); }
	void Push(std::shared_ptr<Job> _job, bool _pushOnly = false);
	void Execute();

private:
	LockQueue<std::shared_ptr<Job>>		jobs;
	std::atomic<int32>					jobCount = 0;
};
