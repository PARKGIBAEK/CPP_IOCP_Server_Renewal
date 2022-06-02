#pragma once

#include <iostream>
#include <filesystem>
#include <source_location>
#include <thread>
#include <chrono>
#include "Lock.h"

#define OUT
#define NAMESPACE_BEGIN(name)	namespace name {
#define NAMESPACE_END(name)		}

/*-------------------
*		LOCK
-------------------*/
// Lock 변수 선언
#define USE_N_LOCKS(count)	Lock locks[count]
#define USE_LOCK			USE_N_LOCKS(1)
// Scope내 LockGuard
#define READ_LOCK_IDX(idx)	ReadLockGuard readLockGuard_##idx(locks[idx],typeid(this).name());
#define READ_LOCK			READ_LOCK_IDX(0)
#define WRITE_LOCK_IDX(idx)	WriteLockGuard writeLockGuard_##idx(locks[idx],typeid(this).name());
#define WRITE_LOCK			WRITE_LOCK_IDX(0)



/*-------------------
*		CRASH
-------------------*/
#if _DEBUG

#define ASSERT_CRASH(expression, message) \
	if (expression) \
	{ \
		const std::source_location sl = std::source_location::current();		\
		std::cout << "Error Occurred :\n>> File location : " << sl.file_name() << "\n"	\
			<< ">> Line : " << sl.line() << "\n"								\
			<< ">> Column : " << sl.column() << "\n"							\
			<< ">> Error Description : " << message << std::endl;				\
		std::terminate();														\
	}																			

#else
#define ASSERT_CRASH(expression, message) 
#endif

#define FORCED_CRASH(message) \
	{ \
		const std::source_location sl = std::source_location::current();		\
		std::cout << "Error :\n>> File location : " << sl.file_name() << "\n"	\
			<< ">> Line : " << sl.line() << "\n"								\
			<< ">> Column : " << sl.column() << "\n"							\
			<< ">> Description : " << message << std::endl;						\
		std::terminate();														\
	}

#define FORCED_CRASH2	do{int*ptr =nullptr; *ptr =100;}while(false)
