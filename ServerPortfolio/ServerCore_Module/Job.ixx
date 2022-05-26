module;
#include <functional>
#include <memory>
#include <type_traits>
#include <cassert>
export module Job;

import Types;


export class Job
{
public:
	
	Job(CallbackType&& _callback) :callback(std::move(_callback))
	{}
	 
	template<typename T, typename Ret, typename... Args>
	Job(std::shared_ptr<T> _owner, Ret(T::* _memFunc)(Args...), Args&&... _args)
	{
		static_assert(std::is_member_function_pointer_v<T::_memFunc>, "_memFunc is not a member function.");
		//assert(std::is_member_function_pointer_v<T::_memFunc>);
		
		// 수행할 작업(함수)에 대한 객체르 callback변수에 전달
		callback = [_owner, _memFunc, _args...]()
		{
			(_owner.get()->*_memFunc)(_args...);
		};
	}

	void Excute()
	{
		callback();
	}
private:
	CallbackType callback;
};