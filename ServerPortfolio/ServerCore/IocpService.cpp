#include "Core.h"
#include "IocpService.h"
#include "IocpEvent.h"

IocpService::IocpService()
{
	iocpHandle = ::CreateIoCompletionPort(INVALID_HANDLE_VALUE, 0, 0, 0);
	ASSERT_CRASH(INVALID_HANDLE_VALUE == iocpHandle, "Failed - Creating a IOCP");
}

IocpService::~IocpService()
{
	::CloseHandle(iocpHandle);
}

//bool IocpService::RegisterSockToIOCP(std::shared_ptr<ISession> _sessionObject)
bool IocpService::RegisterSockToIOCP(SOCKET _socket)
{

	//return ::CreateIoCompletionPort((HANDLE)_sessionObject->GetSocket(), iocpHandle, 0, 0);
	return ::CreateIoCompletionPort((HANDLE)_socket, iocpHandle, 0, 0);
}

bool IocpService::DispatchCompletionPacket(uint32 _timeoutMs)
{
	DWORD bytesTransferred = 0;
	ULONG_PTR key = 0;
	IocpEvent* iocpEvent = nullptr;

	if (true == ::GetQueuedCompletionStatus(
		iocpHandle, &bytesTransferred, &key, (LPOVERLAPPED*)&iocpEvent, _timeoutMs))
	{
		std::shared_ptr<ISession> sessionObject = iocpEvent->ownerSession;

		// DispatchEvent를 호출하는 sessionObject는 Listener 또는 ~Session
		sessionObject->DispatchEvent(iocpEvent, bytesTransferred);
	}
	else
	{
		int32 errorCode = ::WSAGetLastError();
		switch (errorCode)
		{
		case WAIT_TIMEOUT:
			return false;
		default:
			// sessionObject는 Listener 또는 ~Session
			std::shared_ptr<ISession> sessionObject = iocpEvent->ownerSession;
			sessionObject->DispatchEvent(iocpEvent, bytesTransferred);
			break;
		}
	}

	return true;
}
