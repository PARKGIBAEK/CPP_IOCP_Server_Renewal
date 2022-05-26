#include "Core.h"
#include "CoreTLS.h"

thread_local uint32				tls_ThreadId = 0;
thread_local uint64				tls_EndTickCount = 0;
thread_local std::stack<int32>	tls_LockStack;
thread_local JobQueue* tls_CurrentJobQueue = nullptr;
thread_local std::shared_ptr<SendBufferChunk> tls_SendBufferChunk;