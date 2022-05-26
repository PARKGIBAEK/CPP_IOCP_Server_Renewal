#pragma once

struct false_template
{
	static constexpr bool value = false;
};

struct true_template
{
	static constexpr bool value = true;
};

template<typename T>
struct is_shared_ptr : false_template
{};

template<typename T>
struct is_shared_ptr<std::shared_ptr<T>> : true_template
{};

template <typename Type>
class LockQueue
{
	static_assert(is_shared_ptr<Type>::value, "Type must be shared_ptr");
public:
	void Push(Type _item)
	{
		WRITE_LOCK;
		items.push(_item);
	}
	Type Pop()
	{
		WRITE_LOCK;
		if (items.empty())
			return Type();

		Type ret = items.front();
		items.pop();
		return ret;
	}

	void PopAll(Vector<Type>& _items)
	{
		WRITE_LOCK;
		while (Type item = Pop())
			_items.push_back(item);
	}

	Type Peek()
	{
		return items.front();
	}

	void Clear()
	{
		WRITE_LOCK;
		items = Queue<Type>();
	}
private:
	USE_LOCK;
	Queue<Type> items;
};

