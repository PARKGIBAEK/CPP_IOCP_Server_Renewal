module;
#include <memory>
#include "WinsockPack.h"
#include "CoreMacros.h"
module IOCP_Core;
import Types;
import IOCP_Event;

class IOCP_Object :public std::enable_shared_from_this<IOCP_Object>
{
public:
	virtual HANDLE GetHandle() = 0;
	virtual void Dispatch(IOCP_Event* _iocpEvent, int32 _bytesTransferred = 0) = 0;
};

class IOCP_Core
{
public:
	IOCP_Core()
	{
		iocpHandle = ::CreateIoCompletionPort(INVALID_HANDLE_VALUE, 0, 0, 0);
		ASSERT(iocpHandle != INVALID_HANDLE_VALUE, "Failed to create IOCP");
	}
	~IOCP_Core()
	{
		::CloseHandle(iocpHandle);
	}

	bool Register(std::shared_ptr<IOCP_Object> _iocpObject)
	{ // 소켓을 IOCP에 등록
		return ::CreateIoCompletionPort(_iocpObject->GetHandle(), iocpHandle, /*key*/0, 0);
	}

	bool Dispatch(uint32 _timeoutMs)
	{ // IOCP에서 Completion Packet 가져오기
		DWORD bytesTransferred = 0;
		ULONG_PTR key = 0;
		IOCP_Event* iocpEvent = nullptr;

		if ( ::GetQueuedCompletionStatus(
			iocpHandle, &bytesTransferred, &key,
			reinterpret_cast<LPOVERLAPPED*>(&iocpEvent), _timeoutMs) )
		{
			auto iocpObject = iocpEvent->owner;
			iocpObject->Dispatch( iocpEvent, bytesTransferred );
		}
		else
		{
			int32 errCode = ::WSAGetLastError();
			switch (errCode)
			{
			case WAIT_TIMEOUT:
				return false;
			default:
				auto iocpObject = iocpEvent->owner;
				iocpObject->Dispatch(iocpEvent, bytesTransferred);
				break;
			}
		}
		return true;
	}
private:
	HANDLE		iocpHandle;
};