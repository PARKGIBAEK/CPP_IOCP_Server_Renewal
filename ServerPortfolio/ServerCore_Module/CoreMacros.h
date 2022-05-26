#pragma once
#include <chrono> 
#include <thread>
using namespace std::chrono_literals;

#define USE_N_LOCKS(count)	class Lock locks[count];
#define USE_LOCK			USE_N_LOCKS(1)
/* typeid(this).name()�� class�� �̸��� ������ ����̴�.
  DeadlockProfiler���� Lock�� ������ Ŭ������ �ĺ��ϱ� ���� ���
* */
#define READ_LOCK_IDX(idx)	class ReadLockGuard readLockGurad_##idx(locks[idx], typeid(this).name());
#define WRITE_LOCK_IDX(idx)	class WriteLockGuard writeLockGuard_##idx(locks[idx],typeid(this).name());
#if _DEBUG
#define READ_LOCK			READ_LOCK_IDX(0)	\
							std::this_thread::sleep_for(10ms); // ����� �߻��ϴ��� ������ ����Ȯ���ϰ� ���� ��

#define WRITE_LOCK			WRITE_LOCK_IDX(0)	\
							std::this_thread::sleep_for(10ms);
#else
#define READ_LOCK			READ_LOCK_IDX(0)
#define WRITE_LOCK			WRITE_LOCK_IDX(0)
#endif



#include <iostream>
#include <filesystem>
#include <source_location>
//#include <functional>

#ifdef _DEBUG
#define ASSERT(expression, message) \
	if (expression) \
	{ \
		const std::source_location sl = std::source_location::current();		\
		std::cout << "Error Occurred :\n>> File location : " << sl.file_name() << "\n"	\
			<< ">> Line : " << sl.line() << "\n"								\
			<< ">> Column : " << sl.column() << "\n"							\
			<< ">> Error Description : " << message << std::endl;					\
		std::terminate();														\
	}																			


/*===============================================================================*/


#define CRASH(message) \
	{ \
		const std::source_location sl = std::source_location::current();		\
		std::cout << "Error :\n>> File location : " << sl.file_name() << "\n"	\
			<< ">> Line : " << sl.line() << "\n"								\
			<< ">> Column : " << sl.column() << "\n"							\
			<< ">> Description : " << message << std::endl;					\
		std::terminate();														\
	}																			
#else
#define ASSERT(expression, message) 
#define CRASH(message)
#endif