module;
#include <mutex>
#include <atomic>
#include <functional>
export module Types;




export using BYTE = unsigned char;

export using int8 = __int8;
export using int16 = __int16;
export using int32 = __int32;
export using int64 = __int64;

export using uint8 = unsigned __int8;
export using uint16 = unsigned  __int16;
export using uint32 = unsigned  __int32;
export using uint64 = unsigned  __int64;

export unsigned int uint64Max = 0xff'ff'ff'ff'ff'ff'ff'ff;
export int int64Max = uint64Max / 2;
export int int64Min = -int64Max - 1;

export template<typename T>
inline constexpr uint16 Size16(T t)
{
	return static_cast<uint16>(sizeof(T));
}

export template<typename T>
inline constexpr uint32 Size32(T t)
{
	return static_cast<uint32>(sizeof(T));
}

export template<typename T>
inline constexpr uint64 Size64(T t)
{
	return static_cast<uint64>(sizeof(T));
}

export template<typename T>
inline constexpr uint16 Len16(T t)
{
	return static_cast<uint16>(sizeof(t) / sizeof(t[0]));
}

export template<typename T>
inline constexpr uint32 Len32(T t)
{
	return static_cast<uint32>(sizeof(t) / sizeof(t[0]));
}


export template<typename T>
inline constexpr uint64 Len64(T t)
{
	return static_cast<uint64>(sizeof(t) / sizeof(t[0]));
}


export template<typename T>
using Atomic = std::atomic<T>;

export using Mutex = std::mutex;

export using CondVar = std::condition_variable;

export using CallbackType = std::function<void()>;
//#define	USING_SHARED_PTR(name)		using name##_Ref = std::shared_ptr<class name>;

//USING_SHARED_PTR();

