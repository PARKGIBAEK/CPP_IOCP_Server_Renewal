module;
#include <stack>
export module TLS;
import Types;


export extern thread_local uint32 tls_ThreadId;
export extern thread_local uint64 tls_EndTickCount;
export extern thread_local std::stack<uint32> tls_LockStack;
//export thread_local SendBufferChunkRef tls_SendBufferChunk;
//export thread_local JobQueue* tls_CurrentJobQueue = nullptr;
