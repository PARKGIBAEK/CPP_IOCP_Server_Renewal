export module Global;

import DeadlockProfiler;
import ThreadManager; 
import MemoryManager;
// Cyclic Dependency 문제가 발생할 경우를 대비하여 외부 초기화

export extern class DeadlockProfiler* gDeadlockProfiler;
export extern class ThreadManager* gThreadManager; 
export extern class MemoryManager* gMemoryManager;
//class MemoryManager* GMemory;.
//class SendBufferManager;
//class GlobalQueue;
//class JobTimer;


