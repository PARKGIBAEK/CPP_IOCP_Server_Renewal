module;
#include <stack>
#include <map>
#include <vector>
#include <set>
#include <unordered_map>
#include <mutex>
#include <stdio.h>
#include "CoreMacros.h"
export module DeadlockProfiler;

import Types;
import TLS;

/*
�� �޼��� ���� ���
PushNode : ���� Thread���� ���� Lock�� tls_LockStack�� ����(TLS�̹Ƿ� �����庰�� ����)

PopNode : ���� Thread���� Lock�� ��ȯ�� �� tls_LockStack���� Pop����

CheckCycle : 
*/

export class DeadlockProfiler
{
public:
	DeadlockProfiler() {}

	/* _name �� typeid(this).name()�� �����ϱ� ������ ȣ���ڵ尡 ��ġ�� class���� �ȴ�.
	   ���� Lock�� ����ϴ� class�� ���Ͽ� �������� �ν��Ͻ��� �����Ѵٸ�, �ν��Ͻ��� �ĺ��� �� �ִ� ������� _name�� �����ؾ��Ѵ�.
	 * Node�� Lock�� �ǹ�.
	 */
	void PushNode(const char* _name)
	{
		std::lock_guard guard(mtx);

		uint32  CurrentNodeId = 0;

		auto findIt = nameToId.find(_name);
		if (findIt == nameToId.end()) // �߰ߵ� ��� �� _name�� ���� ���(���ʷ� �߰ߵ� ����� ��� ���)
		{
			CurrentNodeId = nameToId.size(); // ��尡 �߰ߵ� ������ ID�� �߱�
			// ó�� �߰ߵ� ����̹Ƿ� ���
			nameToId[_name] = CurrentNodeId; // ����� �̸�(class��)�� ����� ��ȣ(CurrentNodeId)�� ����
			idToName[CurrentNodeId] = _name; // ����� ��ȣ(CurrentNodeId)�� ����� �̸�(class��)�� ����
		}
		else
		{
			CurrentNodeId = findIt->second; // �� ���̵� ����
		}

		// ���� Thread���� ������ ��� �ִ� ���� �ִ� ������ ���( Hold & Wait )
		if (tls_LockStack.empty() == false)
		{
			uint32 prevNodeId = tls_LockStack.top(); // �ٷ� ������ �湮�� ���(Lock) ��ȸ
			if (CurrentNodeId != prevNodeId) // �ٷ� ������ �湮�� ���� ���� �湮�� ��尡 ���� �ʴٸ�(��������� Lock�� ���� ��찡 �ƴ϶��)
			{
				std::set<uint32>& history = lockHistory[prevNodeId]; // prevNodeId���� ���۵� ��� ��ȸ
				if (history.find(CurrentNodeId) == history.end()) // prevNodeId���� CurrentNodeId�� �湮������ ���ٸ�(ó�� �߰ߵ� Lock�̶�� Lock�̷¿� �߰�)
				{
					history.insert(CurrentNodeId); // �̷¿� �߰��ϰ�
					CheckCycle(); // Cycle ���� ����
				}
			}
		}

		tls_LockStack.push(CurrentNodeId); // tls_LockStack�� Thread ���� ���� ��� �ִ� Lock ���� ���
	}

	void PopNode(const char* _name)
	{
		std::lock_guard guard(mtx);

		//Assert([&] {return tls_LockStack.empty(); }, "MULTIPLE_UNLOCK");
		ASSERT(tls_LockStack.empty(), "MULTIPLE_UNLOCK");

		const int32 lockId = nameToId[_name];
		//Assert([&] {return tls_LockStack.top() != CurrentNodeId; }, "INVALID_UNLOCK");
		ASSERT(tls_LockStack.top() != lockId, "INVALID_UNLOCK");

		tls_LockStack.pop();
	}

	void CheckCycle()
	{
		// Cycle Ȯ�� �� �ʿ��� �ӽ� ������ �ʱ�ȭ
		const uint32 NodeCount = nameToId.size(); // ���ݱ��� �߰ߵ� ��� ����
		visitedOrder = std::vector<uint32>(NodeCount, -1); // ��� �湮 ��� �� �迭 �ʱ�ȭ(�湮 ����&�湮 ���� ���)
		visitedCount = 0; // DFS���� �湮�� ������ ����ϱ� ���� ����(������� �湮�� ����� ����)
		isVisited = std::vector<bool>(NodeCount, false); // ���� ��� N �� ���Ͽ� DFS Ž�� �湮 ���� ��� ( isVisited[N] == true �̸� ���N���� ���۵Ǵ� ��� ��� Ȯ�� �Ϸ��� ��)
		prevNodes = std::vector<uint32>(NodeCount, -1); // �湮 ��� ���( N��° ��忡 �湮�ϱ� �� ��ġ�� prevNodes[N] )

		// ��� ���(Lock)�� ���Ͽ� DFS Ȯ��
		for (uint32 lockId = 0; lockId < NodeCount; lockId++)
			DFS(lockId);

		// Cycle Ȯ���� �������Ƿ� �ӽ� �����͸� ����
		visitedOrder.clear();
		isVisited.clear();
		prevNodes.clear();
		visitedCount = 0; // �߰��� �ڵ�(�׽�Ʈ ���غ����Ƿ� �׽�Ʈ�Ͽ� ���� �߻� �� �� �� ����)
	}

	void DFS(uint32 _currentNode)
	{
		if (visitedOrder[_currentNode] != -1) // �̹� �湮�� ����� ��ŵ
			return;

		visitedOrder[_currentNode] = visitedCount++; // _currentNode��忡 �湮�� ������ ���

		// ��� ������ ��� ��ȸ
		auto findIt = lockHistory.find(_currentNode);// _currentNode���� ������ ��� ��ȸ
		if (findIt == lockHistory.end())// _currentNode���� �ٸ� ��带 �湮�� �̷��� ���� ���(_currentNode�� ���� ���¿��� �� �ٸ� Lock�� ���� ���� ���� ���)
		{ // �湮�� �� �����Ƿ� Ž�� �Ϸ� ó���ϰ� ����
			isVisited[_currentNode] = true;
			return;
		}

		std::set<uint32>& history = findIt->second;//
		for (uint32 nextNode : history) // _currentNode�� ���������� �Ͽ� �̾ �湮�ߴ� ��� ��忡 ���Ͽ� DFS
		{
			if (visitedOrder[nextNode] == -1) // nextNode�� �湮�� ���� ���� ��� �߰� ��
			{
				prevNodes[nextNode] = _currentNode; // nextNode�� �湮�ϱ� ������ �湮�� ���� _currentNode���� ���
				DFS(nextNode);
				continue;
			}

			// _currentNode�� nextNode���� ���� �湮�Ǿ��ٸ� ������ ������ �ǹ��ϹǷ� ���� �߻� ������ ���� ������ �н�
			if (visitedOrder[_currentNode] < visitedOrder[nextNode])
				continue;

			// �������� �ƴ� ���, DFS(nextNode)�� ���� ������� �ʾҴٸ�? nextNode�� _currentNode���� ���� �湮�� ���̹Ƿ� ����� ������ �ǹ�
			if (isVisited[nextNode] == false)
			{ // Cycle�� �����ϴ� ����̹Ƿ� CRASH �߻� ��Ű��

				// ����� �߻��� ���� ���
				printf("Found cycle : %s -> %s \n", idToName[_currentNode], idToName[nextNode]); // � ���(Lock �̸�)���� Cycle�� �߻��ߴ��� ���

				uint32 now = _currentNode;
				while (true)
				{ // ����� �߻��� ��� Cycle ������� ����ϱ�
					printf("%s -> %s \n", idToName[prevNodes[now]], idToName[now]); // ������ ���(now)�� �̸��� ��ȣ ���
					now = prevNodes[now]; // now�� ���� ���(Lock) ��ȣ�� ����
					if (now == nextNode) // ����Ŭ�� �ѹ��� �� ���Ҵٸ�
						break;
				}
				//Crash("DEADLOCK_DETECTED");
				CRASH("DEADLOCK_DETECTED");
			}
		}
		isVisited[_currentNode] = true; // ��� _here�� ���� DFS �Ϸ� ���
	}
private:
	/* ������ ���(Lock)�� �ĺ��ϱ� ���� ������
	�� ����� ���� ó�� �߰ߵ� ������ �ѹ��� ����ϹǷ� ��ü Lock�� ������ �ʰ��� �� ����*/
	std::unordered_map<const char*, uint32>	nameToId; // Lock�� �̸��� ��ȣ�� ����
	std::unordered_map<uint32, const char*>	idToName; // Lock�� ��ȣ�� �̸��� ����

	std::map<uint32, std::set<uint32>>		lockHistory; /* key(uint32) : ���ʷ� ���� Lock, 
															value(set<uint32>) : ���ʷ� ���� Lock(key)�� �̾ ���� Lock���� ���
															��) 0�� Lock�� ���ʷ� ���� ���¿��� 1�� Lock, 2�� Lock�� ��Ҵٸ� set���� 1,2�� ������� */

	std::mutex								mtx;
	std::vector<uint32>						visitedOrder; // ��尡 �߰ߵ� ������ ���(CycleȮ�� �� �Ź� �ʱ�ȭ ��)
	uint32									visitedCount = 0; // ��尡 �߰ߵ� ������ �����ϱ� ���� ī��Ʈ ����
	std::vector<bool>						isVisited; // DFS�� ��ü �� Ž�� �� �� n��° Lock�� ���� DFS�� ����Ǿ������� ���
	std::vector<uint32>						prevNodes; // ���� Lock�� ��� ������ Lock�� ���

	// Thread ���� ���� ��� �ִ� Lock ���� ����ϱ� ���� ������ thread_local std::stack<uint32> tls_LockStack �� ���� ��
};