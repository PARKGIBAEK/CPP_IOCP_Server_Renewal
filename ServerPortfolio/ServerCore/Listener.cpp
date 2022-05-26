#include "Core.h"
#include "Listener.h"
#include "IocpService.h"
#include "IocpEvent.h"
#include "SocketHelper.h"
#include "Service.h"
#include "Session.h"
Listener::~Listener()
{
	SocketHelper::CloseSocket(listenSocket);

	for (auto acceptEvent : acceptEvents)
	{
		XDelete(acceptEvent);
	}
}

SOCKET Listener::GetSocket()
{
	return listenSocket;
}

void Listener::DispatchEvent(IocpEvent* _iocpEvent, int32 _byteTransferred)
{
	ASSERT_CRASH(_iocpEvent->eventType != EventType::ACCEPT, "Listener - EventType must be EventType::ACCEPT");
	
	AcceptEvent* acceptEvent = static_cast<AcceptEvent*>(_iocpEvent);

	ProcessAccept(acceptEvent);
}

bool Listener::StartAccept(std::shared_ptr<class ServerService> _service)
{
	ownerService = _service;
	if (_service == nullptr)
		return false;

	listenSocket = SocketHelper::CreateSocket();
	if (listenSocket == INVALID_SOCKET)
		return false;

	if (false == ownerService->GetIocpService()->RegisterSockToIOCP(this->GetSocket()))
		return false;

	if (false == SocketHelper::SetReuseAddress(listenSocket, true))
		return false;

	if (false == SocketHelper::SetLinger(listenSocket, 0, 0))
		return false;

	if (false == SocketHelper::Bind(listenSocket, ownerService->GetNetAddress()))
		return false;

	if (false == SocketHelper::Listen(listenSocket))
		return false;

	// 최대 동접수 만큼 AcceptEvent생성 & RegisterAccept호출
	const int32 acceptCount = ownerService->GetMaxSessionCount();
	for (int32 i = 0; i < acceptCount; i++)
	{
		/* IOCP 사용을 위한 OVERLAPPED 구조체를 상속받은 AcceptEvent를
			MaxSessionCount만큼 미리 등록해두기*/
		AcceptEvent* acceptEvent = XNew<AcceptEvent>();

		acceptEvent->ownerSession = shared_from_this();
		acceptEvents.emplace_back(acceptEvent);
		RegisterAccept(acceptEvent);
	}

	return true;
}

void Listener::CloseSocket()
{
	SocketHelper::CloseSocket(listenSocket);
}

void Listener::RegisterAccept(AcceptEvent* _acceptEvent)
{
	// 접속한 클라 세션 생성하기
	std::shared_ptr<Session> session = ownerService->CreateSession();

	_acceptEvent->InitOverlapped();
	_acceptEvent->ownerSession = session;

	DWORD bytesReceived = 0;
	if (false ==
		SocketHelper::AcceptEx(listenSocket, session->GetSocket(),
			session->recvBuffer.WriteCursor(), 0, sizeof(SOCKADDR_IN) + 16,
			sizeof(SOCKADDR_IN) + 16, &bytesReceived,
			static_cast<LPOVERLAPPED>(_acceptEvent)))
	{
		const int32 errorCode = ::WSAGetLastError();
		if (errorCode != WSA_IO_PENDING)
		{/* 비동기이므로 I/O PENDING 아닌 경우
		 비동기 Accept가 실패한 것이므로 RegisterAccept를 다시 걸어준다.
		 
		 만약 AcceptEx가 true를 반환하거나 WSA_IO_PENDING일 경우
		 IOCP에 Completion Packet으로 도착하기 때문에
		 GQCS함수를 호출하는 스레드 쪽에서 Tail call로 RegisterAccept를 다시 호출해주어
		 Accept를 무한 반복할 수 있게 된다*/
			RegisterAccept(_acceptEvent);
		}
	}
}

void Listener::ProcessAccept(AcceptEvent* _acceptEvent)
{
	std::shared_ptr<Session> session =
		static_pointer_cast<Session>(_acceptEvent->ownerSession);
	if (false == SocketHelper::SyncSocketContext(session->GetSocket(),
		listenSocket))
	{ // 소켓 옵션 동시화 실패하면 다시 Accept걸기
		RegisterAccept(_acceptEvent);
		return;
	}

	SOCKADDR_IN sockAddr;
	int32 addrLen = sizeof(sockAddr);
	if (SOCKET_ERROR != ::getpeername(session->GetSocket(),
		reinterpret_cast<SOCKADDR*>(&sockAddr), &addrLen))
	{ // 주소 가져오기 실패하면 뭔가 잘못된 것이므로 다시 Accept걸기
		RegisterAccept(_acceptEvent);
		return;
	}

	session->SetNetAddress(NetAddress(sockAddr));
	session->ProcessConnect();
	RegisterAccept(_acceptEvent);// AcceptEvent 재활용
}
