module;
#include "WinsockPack.h"
#include <memory>
export module IOCP_Event;
import Types;
import IOCP_Core;

enum class EventType :uint8
{
	Connect,
	Disconnect,
	Accept,
	//PreRecv,
	Recv,
	Send,
};


export class IOCP_Event : public OVERLAPPED
{
public:
	IOCP_Event(EventType _type) :eventType(_type)
	{
		OVERLAPPED::Internal = 0;
		OVERLAPPED::InternalHigh = 0;
		OVERLAPPED::Offset = 0;
		OVERLAPPED::OffsetHigh = 0;
		OVERLAPPED::hEvent = 0;
	}

	EventType  eventType;
	std::shared_ptr<class  IOCP_Object>	owner;
};

export class ConnectEvent :public IOCP_Event
{
public:
	ConnectEvent() :IOCP_Event(EventType::Connect)
	{	}
};

export class DisconnectEvent :public IOCP_Event
{
public:
	DisconnectEvent() :IOCP_Event(EventType::Disconnect)
	{	}
};

export class AcceptEvent :public IOCP_Event
{
public:
	AcceptEvent() :IOCP_Event(EventType::Accept)
	{	}
};

export class RecvEvent :public IOCP_Event
{
public:
	RecvEvent() :IOCP_Event(EventType::Recv)
	{	}
};

export class SendEvent :public IOCP_Event
{
public:
	SendEvent() :IOCP_Event(EventType::Send)
	{	}
};