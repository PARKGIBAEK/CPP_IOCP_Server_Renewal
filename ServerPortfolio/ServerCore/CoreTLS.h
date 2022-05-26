#pragma once
#include <stack>

extern thread_local uint32				tls_ThreadId;
extern thread_local uint64				tls_EndTickCount;
extern thread_local std::stack<int32>	tls_LockStack;
extern thread_local class JobQueue*		tls_CurrentJobQueue;
extern thread_local class std::shared_ptr<class SendBufferChunk> tls_SendBufferChunk;

