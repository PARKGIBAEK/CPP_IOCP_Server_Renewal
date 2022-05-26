#pragma once
#include <stack>
#include <map>
#include <vector>

class DeadLockProfiler
{
public:
	void PushLock(const char* _name);
	void PopLock(const char* _name);
	void CheckCycle();

private:
	void DFS(int32 _current);

private:
	std::unordered_map<const char*, int32>	nameToId;
	std::unordered_map<int32, const char*>	idToName;

	/* 노드(currentLockId)와 연결되어있는 노드(Lock) 목록.
	   즉, 하나의 Thread에서 2개 이상의 Lock을 점유하고 있는 경우에 생성 됨	*/
	std::map<int32, set<int32>>				lockHistory;

	std::mutex mtxLock;

	std::vector<int32>	visitedOrder; // 노드(lock)이 몇번째로 발견되었는지를 기록

	int32				totalNodeCount = 0; // 현재까지 발견된 노드(lock) 갯수

	/*
	   isFinished[3] 이면 lockId가 3인 노드를 DFS의 시작점으로 하여 탐색 완료 여부 기록
	*/
	std::vector<bool>	isFinished; 
	
	/* 1번 락 -> 3번 -> 4번 락을 잡았다면
	* parent[4] = 3
	* parent[3] = 1
	* parent[1] = -1
	*/
	std::vector<int32>	parent;

};

