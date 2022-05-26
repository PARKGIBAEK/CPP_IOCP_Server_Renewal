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

	/* ���(currentLockId)�� ����Ǿ��ִ� ���(Lock) ���.
	   ��, �ϳ��� Thread���� 2�� �̻��� Lock�� �����ϰ� �ִ� ��쿡 ���� ��	*/
	std::map<int32, set<int32>>				lockHistory;

	std::mutex mtxLock;

	std::vector<int32>	visitedOrder; // ���(lock)�� ���°�� �߰ߵǾ������� ���

	int32				totalNodeCount = 0; // ������� �߰ߵ� ���(lock) ����

	/*
	   isFinished[3] �̸� lockId�� 3�� ��带 DFS�� ���������� �Ͽ� Ž�� �Ϸ� ���� ���
	*/
	std::vector<bool>	isFinished; 
	
	/* 1�� �� -> 3�� -> 4�� ���� ��Ҵٸ�
	* parent[4] = 3
	* parent[3] = 1
	* parent[1] = -1
	*/
	std::vector<int32>	parent;

};

