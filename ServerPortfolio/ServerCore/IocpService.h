#pragma once
#include <WinBase.h>

/*==============
	ISession
==============*/
/* Every class carrying out 'Overlapped I/O', like the Listener or Session, inherit this abstract class.
The function 'DispatchEvent()' processes the event corresponding to the purpose of its concrete class.*/
class ISession :public std::enable_shared_from_this<ISession>
{
public:
	virtual SOCKET GetSocket() = 0;
	virtual void DispatchEvent(
		class IocpEvent* _iocpEvent, int32 _byteTransferred = 0) = 0;

};


/*==================
	IocpService
==================*/
// Dispatch each IOCP event through a function call 'GetQueuedCompletionStatus'
class IocpService
{
public:
	IocpService();
	~IocpService();

	HANDLE		GetIocpHandle() { return iocpHandle; }

	//bool		RegisterSockToIOCP(std::shared_ptr<ISession> _sessionObject);
	bool		RegisterSockToIOCP(SOCKET _socket);
	// Call 'GetQueuedCompletionStatus' -> parsing each event -> tail call 'ISession::DispatchCompletionPacket()'
	bool		DispatchCompletionPacket(uint32 _timeoutMs = INFINITE);

private:
	HANDLE iocpHandle;

};