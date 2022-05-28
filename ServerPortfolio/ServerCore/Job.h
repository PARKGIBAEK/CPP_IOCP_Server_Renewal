#pragma once
#include <functional>

using CallbackType = std::function<void()>;

class Job
{
public:
	Job(CallbackType&& _callback) :callback(std::move(_callback))
	{}

	template<typename Type,typename RetType, typename... Args>
	Job(std::shared_ptr<Type> _owner, RetType(Type::* _memFunc)(Args...), Args&&... _args)
	{
		callback = [_owner, _memFunc, _args...]()
		{
			(_owner.get()->*_memFunc)(_args...); // _owner°´Ã¼ÀÇ ¸â¹ö ÇÔ¼ö È£Ãâ
		};
	}

	void Execute()
	{
		callback();
	}
private:
	CallbackType callback;
};

