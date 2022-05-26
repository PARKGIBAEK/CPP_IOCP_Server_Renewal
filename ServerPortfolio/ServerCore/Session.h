#pragma once
#include "IocpEvent.h"
#include "IocpService.h"


class Session : public ISession
{
	friend class Listener;
public:
	constexpr static unsigned int BUFFER_SIZE = 0x0001'0000; // 64KB

	/*		Interface of ISession		*/
	virtual SOCKET		GetSocket() override;
	virtual void		DispatchEvent(IocpEvent* _iocpEvent, int32 _byteTransferred = 0) override;

	/*	 Use externally		*/
	void				Send(std::shared_ptr<SendBuffer> _sendBuffer);
	bool				Connect();
	void				Disconnect(const WCHAR* _cause);

	std::shared_ptr<class Service>	GetOwnerService() { return ownerService.lock(); }
	void							SetService(std::shared_ptr<class Service> _service) { ownerService = _service; }
	
	/*		info	*/
	NetAddress						GetNetAddress() { return netAddress; }
	void							SetNetAddress(NetAddress _netAddress) { netAddress = _netAddress; }
	
	std::shared_ptr<Session>		GetSessionRef() { return static_pointer_cast<Session>(shared_from_this()); }

	bool							IsConnected() { return isConnected; }

	/*		I/O Send & Recv		*/
	bool RegisterConnect();// only for 'Client'
	bool RegisterDisconnect();
	void RegisterRecv();
	void RegisterSend();

	void ProcessConnect();
	void ProcessDisconnect();
	void ProcessRecv(int32 _bytesTransferred);
	void ProcessSend(int32 _bytesTransferred);

	void HandleError(int32 _errorCode);
	
protected:
	/*		Redefine in the content class		*/
	virtual void	OnConnected() {};
	virtual int32	OnRecv(BYTE* _buffer, int32 _len) { return _len; }
	virtual void	OnSend(int32 _len) {}
	virtual void	OnDisconnected(){}
private:
	/*		Session basic member		*/
	SOCKET								socket = INVALID_SOCKET;
	NetAddress							netAddress = {};
	std::atomic<bool>					isConnected = false;
	std::weak_ptr<class Service>		ownerService;

	USE_LOCK;
	/*		Recv part		*/
	RecvBuffer							recvBuffer;

	/*		Send part		*/
	Queue<std::shared_ptr<SendBuffer>>	sendQueue;
	std::atomic<bool>					isSendRegistered;

	/*		IocpEvent for recycle		*/
	ConnectEvent						connectEvent;
	DisconnectEvent						disconnectEvent;
	RecvEvent							recvEvent;
	SendEvent							sendEvent;

};

