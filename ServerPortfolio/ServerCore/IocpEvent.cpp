#include "Core.h"
#include "IocpEvent.h"
#include "IocpService.h"
#include "Session.h"
IocpEvent::IocpEvent(EventType _type):eventType(_type)
{
	InitOverlapped();
}

void IocpEvent::InitOverlapped()
{
	//memset(this, 0, sizeof(OVERLAPPED));

	OVERLAPPED::hEvent = 0;
	OVERLAPPED::Internal= 0;
	OVERLAPPED::InternalHigh = 0;
	OVERLAPPED::Offset= 0;
	OVERLAPPED::OffsetHigh= 0;
	//ownerSession = nullptr;
}
