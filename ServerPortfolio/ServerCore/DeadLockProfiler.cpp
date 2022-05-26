#include "Core.h"
#include "DeadLockProfiler.h"

void DeadLockProfiler::PushLock(const char* _name)
{
	std::lock_guard<std::mutex> guard(mtxLock);

	int32 currentLockId = 0;
	
	auto it2lockId = nameToId.find(_name);
	/* _name�� �ش��ϴ� ���� ��ϵ� ����� ���� ���
		������� ��ϵ� ���ҵ��� ������ lockId�� �ȴ�	*/ 
	if (it2lockId == nameToId.cend())
	{
		currentLockId = static_cast<int32>(nameToId.size());
		nameToId[_name] = currentLockId;
		idToName[currentLockId] = _name;
	}
	else
	{// �̹� ��ϵ� ���̶�� _name�� �����Ǵ� currentLockId ������
		currentLockId = it2lockId->second;
	}

	/* ���� ���� Thread���� lockId�� �ش��ϴ� Lock �̿ܿ� �� �ٸ� Lock�� ��� �־��� ���
		(Lock�� 2�� �̻� ��� �ִ� ���)*/
	if (tls_LockStack.empty() == false)
	{
		const int32 prevLockId = tls_LockStack.top(); // �ٷ� ������ ȹ���� Lock ID��ȸ

		// ��� Lock�� �ƴ� ���
		if (currentLockId != prevLockId)
		{
			// prevLockId�� �ڽ� ���(Lock) ��Ͽ� ���� ���(Lock) �߰�
			std::set<int32>& history = lockHistory[prevLockId];
			if (history.find(currentLockId) == history.cend())
			{
				history.insert(currentLockId);
				CheckCycle();
			}
		}
	}

	tls_LockStack.push(currentLockId);
}

void DeadLockProfiler::PopLock(const char* _name)
{
	std::lock_guard<std::mutex> guard(mtxLock);

	if (tls_LockStack.empty())
		FORCED_CRASH("MULTIPLE_UNLOCK");

	uint32 lockId = nameToId[_name];
	if (tls_LockStack.top() != lockId)
		FORCED_CRASH("INVALID_UNLOCK");

	tls_LockStack.pop();
}

void DeadLockProfiler::CheckCycle()
{
	const int32 lockCount = static_cast<int32>(nameToId.size());
	// �׷��� Ž���� ���� �ʱ�ȭ
	visitedOrder = vector<int32>(lockCount, -1);
	totalNodeCount = 0;
	isFinished = vector<bool>(lockCount, false);
	parent = vector<int32>(lockCount, -1);

	// �� Lock ��庰�� DFSŽ��
	for (int32 lockId = 0; lockId < lockCount; lockId++)
		DFS(lockId);
}

void DeadLockProfiler::DFS(int32 _currentLock)
{
	// -1�� �ƴϸ� �̹� �湮�� ���(Lock)
	if (visitedOrder[_currentLock] != -1)
		return;
	// �湮 ���� ���
	visitedOrder[_currentLock] = totalNodeCount++;
	
	// _currentLock���� �湮 ������ ��� ��ȸ(_currentLock �ٷ� ������ ȹ���� Lock ��ȸ )
	auto findIt = lockHistory.find(_currentLock);

	// �湮 ������ ��尡 ���� ��� (_currentLock ������ ȹ���� Lock�� ���� ���)
	if (findIt == lockHistory.cend())
	{// _currentLock�� ó�� �湮�ϴ� ����̹Ƿ� Ž�� �Ϸ� �� ������ ó��
		isFinished[_currentLock] = true;
		return;
	}

	// _currentLock���� �湮 ������ ��尡 �ִ� ���
	set<int32>& nextSet = findIt->second;
	for (int32 nextLock : nextSet)
	{
		if (visitedOrder[nextLock] == -1)// ó�� �湮�ϴ� ���
		{
			// parent[nextLock] �� next���� ��� ������ ���� ���� �ǹ�
			parent[nextLock] = _currentLock; // next���� �湮 ���� _currentLock�� ǥ��
			DFS(nextLock);
			continue;
		}

		// _currentLockd�� nextLock���� ���� �湮�� ����� ������ ����
		if (visitedOrder[_currentLock] < visitedOrder[nextLock])
			continue;

		/* ���� ������ �������� �ʰ� ������� �´ٴ� ���� �̹� �湮�� ��带 ��湮 �Ѵٴ� �ǹ��̴�.
			��, nextLock�� ���� DFS�� ������ ������ �ʾҴµ� �ٽ� nextLock���� ���ƿԴٴ� ����
			nextLock���� ���ƿ� ���� Cycle�� �ϼ��� ���� �ǹ��Ѵ�.
			���� DFS�� �Ϸ�Ǿ��ٸ� ���ÿ� �����Ǵ� ���� �ƴϹǷ� DeadLock�� �߻����� �ʴ´�*/
		if (isFinished[nextLock] == false)
		{
			// ���� Cycle Ž�� ���� ���
			// ex) Cycle������ 0->1->2->0 �� ��� 2->0 ���
			printf("%s -> %s\n", idToName[_currentLock], idToName[nextLock]);
			int32 now = _currentLock;// 
			while (true)
			{
				// ex) 1->2 , 0->1
				printf("%s -> %s\n", idToName[parent[now]], idToName[now]);
				// �θ� ���� �̵�
				now = parent[now]; // now = 1, now = 0
				if (now == nextLock) // ex) 0->1->2->0 �� ��� 2->0, 1->2, 0->1 �������� ����
					break;
			}

			FORCED_CRASH("DeadLock detected");
		}
	}

	isFinished[_currentLock] = true;


}
