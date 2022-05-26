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

	// �ִ� ������ ��ŭ AcceptEvent���� & RegisterAcceptȣ��
	const int32 acceptCount = ownerService->GetMaxSessionCount();
	for (int32 i = 0; i < acceptCount; i++)
	{
		/* IOCP ����� ���� OVERLAPPED ����ü�� ��ӹ��� AcceptEvent��
			MaxSessionCount��ŭ �̸� ����صα�*/
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
	// ������ Ŭ�� ���� �����ϱ�
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
		{/* �񵿱��̹Ƿ� I/O PENDING �ƴ� ���
		 �񵿱� Accept�� ������ ���̹Ƿ� RegisterAccept�� �ٽ� �ɾ��ش�.
		 
		 ���� AcceptEx�� true�� ��ȯ�ϰų� WSA_IO_PENDING�� ���
		 IOCP�� Completion Packet���� �����ϱ� ������
		 GQCS�Լ��� ȣ���ϴ� ������ �ʿ��� Tail call�� RegisterAccept�� �ٽ� ȣ�����־�
		 Accept�� ���� �ݺ��� �� �ְ� �ȴ�*/
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
	{ // ���� �ɼ� ����ȭ �����ϸ� �ٽ� Accept�ɱ�
		RegisterAccept(_acceptEvent);
		return;
	}

	SOCKADDR_IN sockAddr;
	int32 addrLen = sizeof(sockAddr);
	if (SOCKET_ERROR != ::getpeername(session->GetSocket(),
		reinterpret_cast<SOCKADDR*>(&sockAddr), &addrLen))
	{ // �ּ� �������� �����ϸ� ���� �߸��� ���̹Ƿ� �ٽ� Accept�ɱ�
		RegisterAccept(_acceptEvent);
		return;
	}

	session->SetNetAddress(NetAddress(sockAddr));
	session->ProcessConnect();
	RegisterAccept(_acceptEvent);// AcceptEvent ��Ȱ��
}
