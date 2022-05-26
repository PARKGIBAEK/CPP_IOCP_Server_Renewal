#include "Core.h"
#include "Session.h"
#include "IocpService.h"
#include "Service.h"
#include "SocketHelper.h"
using namespace std;

SOCKET Session::GetSocket()
{
	return socket;
}

void Session::DispatchEvent(IocpEvent* _iocpEvent, int32 _byteTransferred)
{
	switch (_iocpEvent->eventType)
	{
	case EventType::CONNECT:
		ProcessConnect();
		break;
	case EventType::DISCONNECT:
		ProcessDisconnect();
		break;
	case EventType::RECV:
		ProcessRecv(_byteTransferred);
		break;
	case EventType::SEND:
		ProcessSend(_byteTransferred);
		break;
	default:
		break;
	}

}

void Session::Send(std::shared_ptr<SendBuffer> _sendBuffer)
{
	if (IsConnected() == false)
		return;

	bool registerSend = false;

	// RegisterSend()�� �ɸ��� ���� ����
	{
		WRITE_LOCK;

		sendQueue.emplace(_sendBuffer);

		if (isSendRegistered.exchange(true) == false)
			registerSend = true;
	}
	if (registerSend)
		RegisterSend();
}

bool Session::Connect()
{
	return RegisterConnect();
}

void Session::Disconnect(const WCHAR* _cause)
{
}

bool Session::RegisterConnect()
{
	if (IsConnected())
		return false;
	// RegisterConnect�� Ŭ���̾�Ʈ�� 
	if (GetOwnerService()->GetServiceType() != ServiceType::Client)
		return false;

	if (SocketHelper::SetReuseAddress(socket, true) == false)
		return false;

	if (SocketHelper::BindAnyAddress(socket, 0) == false)
		return false;

	connectEvent.InitOverlapped();
	connectEvent.ownerSession = shared_from_this();

	DWORD bytesSent = 0;
	SOCKADDR_IN sockAddr = GetOwnerService()->GetNetAddress().GetSockAddr();
	if (false == SocketHelper::ConnectEx(socket,
		reinterpret_cast<SOCKADDR*>(&sockAddr), sizeof(sockAddr),
		nullptr, 0, &bytesSent, &connectEvent))
	{
		int32 errorCode = ::WSAGetLastError();
		if (WSA_IO_PENDING != errorCode)
		{
			disconnectEvent.ownerSession = nullptr;
			return false;
		}
	}
	return true;
}

bool Session::RegisterDisconnect()
{
	disconnectEvent.InitOverlapped();
	disconnectEvent.ownerSession = shared_from_this();

	/* TE_REUSE_SOCKET �ɼ�
	: ���� �ڵ��� ������ �� �ֵ��� ��.
	  DisconnectEx ��û�� �Ϸ�� ��,
	  ���� �ڵ��� AcceptEx�� ConnectEx�� �Ѱ� ���� �� �� �ְ� ��.
	  */
	if (false == SocketHelper::DisconnectEx(socket, &disconnectEvent,
		TF_REUSE_SOCKET, 0))
	{
		int32 errorCode = ::WSAGetLastError();
		if (WSA_IO_PENDING != errorCode)
		{
			disconnectEvent.ownerSession = nullptr;
			return false;
		}
	}
	return false;
}

void Session::RegisterRecv()
{
	if (false == IsConnected())
		return;

	recvEvent.InitOverlapped();
	recvEvent.ownerSession = shared_from_this();

	WSABUF wsaBuf;
	wsaBuf.buf = reinterpret_cast<CHAR*>(recvBuffer.WriteCursor());
	wsaBuf.len = recvBuffer.FreeSize();

	DWORD bytesReceived = 0;
	DWORD flags = 0;
	if (SOCKET_ERROR != ::WSARecv(socket, &wsaBuf, 1, &bytesReceived, &flags,
		&recvEvent, nullptr))
	{
		int32 errorCode = ::WSAGetLastError();
		if (errorCode != WSA_IO_PENDING)
		{
			HandleError(errorCode);
			recvEvent.ownerSession = nullptr;
		}
	}
}

void Session::RegisterSend()
{
	if (false == IsConnected())
		return;

	sendEvent.InitOverlapped();
	sendEvent.ownerSession = shared_from_this();

	{
		WRITE_LOCK;

		int32 writeSize = 0;
		// sendQueue�� �ִ°� ���δ� sendEvent�� �ֱ�
		while (false == sendQueue.empty())
		{
			std::shared_ptr<SendBuffer> sendBuffer = sendQueue.front();
			writeSize += sendBuffer->WriteSize();
			
			sendQueue.pop();
			sendEvent.sendBuffers.emplace_back(sendBuffer);
		}
	}

	Vector<WSABUF> wsaBufs;
	wsaBufs.reserve(sendEvent.sendBuffers.size());
	for (auto sendBuffer : sendEvent.sendBuffers)
	{
		WSABUF wsaBuf;
		wsaBuf.buf = reinterpret_cast<char*>(sendBuffer->Buffer());
		wsaBuf.len = static_cast<ULONG>(sendBuffer->WriteSize());
	}

}

void Session::ProcessConnect()
{
	connectEvent.ownerSession = nullptr;
	isConnected.store(true);

	// owner ���񽺿� ���� ���� ����ϱ�
	GetOwnerService()->AddSession(GetSessionRef());

	// ���� �� ����(������ �ڵ忡�� ������)
	OnConnected();

	// �񵿱� ���� ���
	RegisterRecv();
}

void Session::ProcessDisconnect()
{
	disconnectEvent.ownerSession = nullptr;

	OnDisconnected();
	GetOwnerService()->ReleaseSession(GetSessionRef());
}

void Session::ProcessRecv(int32 _bytesTransferred)
{
	recvEvent.ownerSession = nullptr;
	if (_bytesTransferred == 0)
	{
		Disconnect(L"Recv 0");
		return;
	}

	if (recvBuffer.OnWrite(_bytesTransferred) == false)
	{
		Disconnect(L"OnWrite overflow");
		return;
	}

	int32 dataSize = recvBuffer.DataSize();

	int32 processLen = OnRecv(recvBuffer.ReadCursor(), dataSize);

	if (processLen < 0 ||
		dataSize < processLen ||
		recvBuffer.OnRead(processLen) == false)
	{
		Disconnect(L"OnRead overflow");
		return;
	}

	recvBuffer.Clean();

	RegisterRecv();// Tail call for Recv
}

void Session::ProcessSend(int32 _bytesTransferred)
{
	//sendEvent.ownerSession = nullptr;
	sendEvent.ownerSession.reset();
	sendEvent.sendBuffers.clear();

	if (_bytesTransferred == 0)
	{
		Disconnect(L"Send 0");
		return;
	}

	OnSend(_bytesTransferred);

	WRITE_LOCK;
	// ������ ������ 
	if (sendQueue.empty())
		isSendRegistered.store(false);
	else
		RegisterSend();
}

void Session::HandleError(int32 _errorCode)
{
}
