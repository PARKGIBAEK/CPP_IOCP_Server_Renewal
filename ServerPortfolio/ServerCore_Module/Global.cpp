module Global; // 모듈 인터페이스 선언부 가져오기 (Global.cpp에서 #include Global.h 하는것과 비슷한 원리)

import DeadlockProfiler;
import ThreadManager;
import MemoryManager;

DeadlockProfiler* gDeadlockProfiler = new DeadlockProfiler();

ThreadManager* gThreadManager = new ThreadManager();

MemoryManager* gMemoryManager = new MemoryManager();