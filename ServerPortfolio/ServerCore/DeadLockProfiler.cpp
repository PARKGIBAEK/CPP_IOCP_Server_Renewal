#include "Core.h"
#include "DeadLockProfiler.h"

void DeadLockProfiler::PushLock(const char* _name)
{
	std::lock_guard<std::mutex> guard(mtxLock);

	int32 currentLockId = 0;
	
	auto it2lockId = nameToId.find(_name);
	/* _name에 해당하는 락이 등록된 기록이 없는 경우
		현재까지 등록된 원소들의 갯수가 lockId가 된다	*/ 
	if (it2lockId == nameToId.cend())
	{
		currentLockId = static_cast<int32>(nameToId.size());
		nameToId[_name] = currentLockId;
		idToName[currentLockId] = _name;
	}
	else
	{// 이미 등록된 락이라면 _name에 대응되는 currentLockId 가져옴
		currentLockId = it2lockId->second;
	}

	/* 만약 현재 Thread에서 lockId에 해당하는 Lock 이외에 또 다른 Lock을 잡고 있었던 경우
		(Lock을 2개 이상 잡고 있는 경우)*/
	if (tls_LockStack.empty() == false)
	{
		const int32 prevLockId = tls_LockStack.top(); // 바로 직전에 획득한 Lock ID조회

		// 재귀 Lock이 아닌 경우
		if (currentLockId != prevLockId)
		{
			// prevLockId의 자식 노드(Lock) 목록에 현재 노드(Lock) 추가
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
	// 그래프 탐색을 위한 초기화
	visitedOrder = vector<int32>(lockCount, -1);
	totalNodeCount = 0;
	isFinished = vector<bool>(lockCount, false);
	parent = vector<int32>(lockCount, -1);

	// 각 Lock 노드별로 DFS탐색
	for (int32 lockId = 0; lockId < lockCount; lockId++)
		DFS(lockId);
}

void DeadLockProfiler::DFS(int32 _currentLock)
{
	// -1이 아니면 이미 방문한 노드(Lock)
	if (visitedOrder[_currentLock] != -1)
		return;
	// 방문 순서 기록
	visitedOrder[_currentLock] = totalNodeCount++;
	
	// _currentLock에서 방문 가능한 노드 조회(_currentLock 바로 다음에 획득한 Lock 조회 )
	auto findIt = lockHistory.find(_currentLock);

	// 방문 가능한 노드가 없는 경우 (_currentLock 다음에 획득한 Lock이 없는 경우)
	if (findIt == lockHistory.cend())
	{// _currentLock이 처음 방문하는 노드이므로 탐색 완료 된 것으로 처리
		isFinished[_currentLock] = true;
		return;
	}

	// _currentLock에서 방문 가능한 노드가 있는 경우
	set<int32>& nextSet = findIt->second;
	for (int32 nextLock : nextSet)
	{
		if (visitedOrder[nextLock] == -1)// 처음 방문하는 경우
		{
			// parent[nextLock] 는 next락을 잡기 직전에 잡은 락을 의미
			parent[nextLock] = _currentLock; // next이전 방문 노드는 _currentLock로 표기
			DFS(nextLock);
			continue;
		}

		// _currentLockd이 nextLock보다 먼저 방문한 노드라면 순방향 간선
		if (visitedOrder[_currentLock] < visitedOrder[nextLock])
			continue;

		/* 위의 조건을 만족하지 않고 여기까지 온다는 것은 이미 방문한 노드를 재방문 한다는 의미이다.
			즉, nextLock에 대한 DFS가 완전히 끝나지 않았는데 다시 nextLock으로 돌아왔다는 것은
			nextLock에서 돌아온 순간 Cycle이 완성된 것을 의미한다.
			만약 DFS가 완료되었다면 동시에 점유되는 락이 아니므로 DeadLock이 발생하지 않는다*/
		if (isFinished[nextLock] == false)
		{
			// 최초 Cycle 탐지 지점 출력
			// ex) Cycle형성이 0->1->2->0 인 경우 2->0 출력
			printf("%s -> %s\n", idToName[_currentLock], idToName[nextLock]);
			int32 now = _currentLock;// 
			while (true)
			{
				// ex) 1->2 , 0->1
				printf("%s -> %s\n", idToName[parent[now]], idToName[now]);
				// 부모 노드로 이동
				now = parent[now]; // now = 1, now = 0
				if (now == nextLock) // ex) 0->1->2->0 인 경우 2->0, 1->2, 0->1 까지오면 끝남
					break;
			}

			FORCED_CRASH("DeadLock detected");
		}
	}

	isFinished[_currentLock] = true;


}
