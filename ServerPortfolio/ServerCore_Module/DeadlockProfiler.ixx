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
각 메서드 동작 요약
PushNode : 현재 Thread에서 잡은 Lock을 tls_LockStack에 삽입(TLS이므로 쓰레드별로 독립)

PopNode : 현재 Thread에서 Lock을 반환할 떄 tls_LockStack에서 Pop수행

CheckCycle : 
*/

export class DeadlockProfiler
{
public:
	DeadlockProfiler() {}

	/* _name 은 typeid(this).name()를 전달하기 때문에 호출코드가 위치한 class명이 된다.
	   만약 Lock을 사용하는 class에 대하여 여러개의 인스턴스가 존재한다면, 인스턴스를 식별할 수 있는 방법으로 _name을 설정해야한다.
	 * Node는 Lock을 의미.
	 */
	void PushNode(const char* _name)
	{
		std::lock_guard guard(mtx);

		uint32  CurrentNodeId = 0;

		auto findIt = nameToId.find(_name);
		if (findIt == nameToId.end()) // 발견된 노드 중 _name이 없는 경우(최초로 발견된 노드인 경우 등록)
		{
			CurrentNodeId = nameToId.size(); // 노드가 발견된 순서를 ID로 발급
			// 처음 발견된 노드이므로 등록
			nameToId[_name] = CurrentNodeId; // 노드의 이름(class명)을 노드의 번호(CurrentNodeId)와 매핑
			idToName[CurrentNodeId] = _name; // 노드의 번호(CurrentNodeId)를 노드의 이름(class명)과 매핑
		}
		else
		{
			CurrentNodeId = findIt->second; // 락 아이디 추출
		}

		// 현재 Thread에서 기존에 잡고 있는 락이 있는 상태인 경우( Hold & Wait )
		if (tls_LockStack.empty() == false)
		{
			uint32 prevNodeId = tls_LockStack.top(); // 바로 직전에 방문한 노드(Lock) 조회
			if (CurrentNodeId != prevNodeId) // 바로 직전에 방문한 노드와 지금 방문한 노드가 같지 않다면(재귀적으로 Lock을 잡은 경우가 아니라면)
			{
				std::set<uint32>& history = lockHistory[prevNodeId]; // prevNodeId에서 시작된 경로 조회
				if (history.find(CurrentNodeId) == history.end()) // prevNodeId에서 CurrentNodeId를 방문한적이 없다면(처음 발견된 Lock이라면 Lock이력에 추가)
				{
					history.insert(CurrentNodeId); // 이력에 추가하고
					CheckCycle(); // Cycle 유무 점검
				}
			}
		}

		tls_LockStack.push(CurrentNodeId); // tls_LockStack은 Thread 별로 현재 잡고 있는 Lock 순서 기록
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
		// Cycle 확인 시 필요한 임시 데이터 초기화
		const uint32 NodeCount = nameToId.size(); // 지금까지 발견된 노드 갯수
		visitedOrder = std::vector<uint32>(NodeCount, -1); // 노드 방문 기록 용 배열 초기화(방문 여부&방문 순서 기록)
		visitedCount = 0; // DFS에서 방문한 순서를 기록하기 위한 변수(현재까지 방문한 노드의 갯수)
		isVisited = std::vector<bool>(NodeCount, false); // 시작 노드 N 에 대하여 DFS 탐색 방문 여부 기록 ( isVisited[N] == true 이면 노드N에서 시작되는 경로 모두 확인 완료한 것)
		prevNodes = std::vector<uint32>(NodeCount, -1); // 방문 경로 기록( N번째 노드에 방문하기 전 위치는 prevNodes[N] )

		// 모든 노드(Lock)에 대하여 DFS 확인
		for (uint32 lockId = 0; lockId < NodeCount; lockId++)
			DFS(lockId);

		// Cycle 확인이 끝났으므로 임시 데이터를 정리
		visitedOrder.clear();
		isVisited.clear();
		prevNodes.clear();
		visitedCount = 0; // 추가한 코드(테스트 안해봤으므로 테스트하여 에러 발생 시 이 줄 삭제)
	}

	void DFS(uint32 _currentNode)
	{
		if (visitedOrder[_currentNode] != -1) // 이미 방문한 노드라면 스킵
			return;

		visitedOrder[_currentNode] = visitedCount++; // _currentNode노드에 방문한 순서를 기록

		// 모든 인접한 노드 순회
		auto findIt = lockHistory.find(_currentNode);// _currentNode에서 시작한 경로 조회
		if (findIt == lockHistory.end())// _currentNode에서 다른 노드를 방문한 이력이 없는 경우(_currentNode을 잡은 상태에서 또 다른 Lock을 잡은 적이 없는 경우)
		{ // 방문할 게 없으므로 탐색 완료 처리하고 종료
			isVisited[_currentNode] = true;
			return;
		}

		std::set<uint32>& history = findIt->second;//
		for (uint32 nextNode : history) // _currentNode을 시작점으로 하여 이어서 방문했던 모든 노드에 대하여 DFS
		{
			if (visitedOrder[nextNode] == -1) // nextNode에 방문한 적이 없는 노드 발견 시
			{
				prevNodes[nextNode] = _currentNode; // nextNode에 방문하기 직전에 방문한 노드는 _currentNode임을 기록
				DFS(nextNode);
				continue;
			}

			// _currentNode가 nextNode보다 먼저 방문되었다면 순방향 간선을 의미하므로 문제 발생 여지가 없기 때문에 패스
			if (visitedOrder[_currentNode] < visitedOrder[nextNode])
				continue;

			// 순방향이 아닌 경우, DFS(nextNode)가 아직 종료되지 않았다면? nextNode가 _currentNode보다 먼저 방문된 것이므로 양방향 간선을 의미
			if (isVisited[nextNode] == false)
			{ // Cycle이 존재하는 경우이므로 CRASH 발생 시키기

				// 데드락 발생한 구간 출력
				printf("Found cycle : %s -> %s \n", idToName[_currentNode], idToName[nextNode]); // 어떤 노드(Lock 이름)에서 Cycle이 발생했는지 출력

				uint32 now = _currentNode;
				while (true)
				{ // 데드락 발생한 경로 Cycle 순서대로 출력하기
					printf("%s -> %s \n", idToName[prevNodes[now]], idToName[now]); // 직전의 노드(now)의 이름과 번호 출력
					now = prevNodes[now]; // now를 직전 노드(Lock) 번호로 변경
					if (now == nextNode) // 사이클을 한바퀴 다 돌았다면
						break;
				}
				//Crash("DEADLOCK_DETECTED");
				CRASH("DEADLOCK_DETECTED");
			}
		}
		isVisited[_currentNode] = true; // 노드 _here에 대한 DFS 완료 기록
	}
private:
	/* 각각의 노드(Lock)를 식별하기 위한 데이터
	※ 참고로 노드는 처음 발견된 시점에 한번만 등록하므로 전체 Lock의 갯수를 초과할 수 없다*/
	std::unordered_map<const char*, uint32>	nameToId; // Lock의 이름과 번호를 매핑
	std::unordered_map<uint32, const char*>	idToName; // Lock의 번호와 이름을 매핑

	std::map<uint32, std::set<uint32>>		lockHistory; /* key(uint32) : 최초로 잡은 Lock, 
															value(set<uint32>) : 최초로 잡은 Lock(key)에 이어서 잡은 Lock들의 목록
															예) 0번 Lock을 최초로 잡은 상태에서 1번 Lock, 2번 Lock을 잡았다면 set에는 1,2가 들어있음 */

	std::mutex								mtx;
	std::vector<uint32>						visitedOrder; // 노드가 발견된 순서를 기록(Cycle확인 시 매번 초기화 됨)
	uint32									visitedCount = 0; // 노드가 발견된 순서를 추적하기 위한 카운트 역할
	std::vector<bool>						isVisited; // DFS로 전체 락 탐색 시 각 n번째 Lock에 대한 DFS가 종료되었는지를 기록
	std::vector<uint32>						prevNodes; // 현재 Lock을 잡기 이전의 Lock을 기록

	// Thread 별로 현재 잡고 있는 Lock 순서 기록하기 위한 데이터 thread_local std::stack<uint32> tls_LockStack 가 존재 함
};