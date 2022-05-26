#pragma once

enum class EventType :uint8
{
	CONNECT,
	DISCONNECT,
	ACCEPT,
	RECV,
	SEND
};

class IocpEvent :public OVERLAPPED
{
public:
	IocpEvent(EventType _type);
	void InitOverlapped();
public:
	EventType eventType;
	std::shared_ptr<class ISession> ownerSession;
};


class ConnectEvent :public IocpEvent
{
public:
	ConnectEvent() :IocpEvent(EventType::CONNECT)	{	}
};

class DisconnectEvent :public IocpEvent
{
public:
	DisconnectEvent() :IocpEvent(EventType::DISCONNECT)	{	}
};

class  AcceptEvent :public IocpEvent
{
public:
	AcceptEvent() :IocpEvent(EventType::ACCEPT)	{	}
public:
	//std::shared_ptr<class Session> ownerSession = nullptr;
};


class RecvEvent :public IocpEvent
{
public:
	RecvEvent() :IocpEvent(EventType::RECV)	{	}
};


class SendEvent :public IocpEvent
{
public:
	SendEvent() :IocpEvent(EventType::SEND)	{	}
public:
	Vector<std::shared_ptr<class SendBuffer>> sendBuffers;
};


