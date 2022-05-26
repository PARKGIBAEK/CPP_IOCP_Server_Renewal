module;
#include <stack>;
module TLS;
import Types;

thread_local uint32 tls_ThreadId = 0;
thread_local uint64 tls_EndTickCount = 0;
thread_local std::stack<uint32> tls_LockStack;
//export thread_local SendBufferChunkRef tls_SendBufferChunk;
//export thread_local JobQueue* tls_CurrentJobQueue = nullptr;
