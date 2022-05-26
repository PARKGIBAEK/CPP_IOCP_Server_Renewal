module Global; // ��� �������̽� ����� �������� (Global.cpp���� #include Global.h �ϴ°Ͱ� ����� ����)

import DeadlockProfiler;
import ThreadManager;
import MemoryManager;

DeadlockProfiler* gDeadlockProfiler = new DeadlockProfiler();

ThreadManager* gThreadManager = new ThreadManager();

MemoryManager* gMemoryManager = new MemoryManager();