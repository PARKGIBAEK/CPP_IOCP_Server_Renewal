module;
#include <queue>
#include <type_traits>
#include "CoreMacros.h"
#define OUT
export module LockQueue;

import Types;
import Lock;

export template<typename T>
class LockQueue
{
public:
	void Push(T _item)
	{
		static_assert(std::is_pointer_v<T>, "T must be pointer type");

		WRITE_LOCK;

		items.push(_item);
		
	}

	T Pop()
	{
		WRITE_LOCK;

		if (items.empty())
			return T();

		T ret = items.front();
		items.pop();
		return ret;
	}

	void PopAll(OUT std::vector<T>& items)
	{
		WRITE_LOCK;

		while (T item = Pop())
			items.emplace_back(std::forward<T>(item));
	}

	void Clear()
	{
		WRITE_LOCK;

		items = std::queue<T>();
	}

private:

	USE_LOCK;

	std::queue<T> items;
};