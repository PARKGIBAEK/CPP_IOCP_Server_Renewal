export module Global;

import DeadlockProfiler;
import ThreadManager; 
import MemoryManager;
// Cyclic Dependency ������ �߻��� ��츦 ����Ͽ� �ܺ� �ʱ�ȭ

export extern class DeadlockProfiler* gDeadlockProfiler;
export extern class ThreadManager* gThreadManager; 
export extern class MemoryManager* gMemoryManager;
//class MemoryManager* GMemory;.
//class SendBufferManager;
//class GlobalQueue;
//class JobTimer;


