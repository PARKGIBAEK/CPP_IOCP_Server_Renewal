#include "Core.h"
#include "Service.h"
#include "Session.h"
#include "IocpService.h"
#include "Listener.h"

Service::Service(ServiceType _type, NetAddress _address,
	std::shared_ptr<IocpService> _iocpService, SessionFactory _factory,
	int32 _maxSessionCount) 
	:serviceType(_type), netAddress(_address), iocpService(_iocpService),
	sessionFactory(_factory), maxSessionCount(_maxSessionCount)
{

}

Service::~Service()
{
}

bool Service::CanStart()
{
	return  nullptr != sessionFactory;
}

void Service::CloseService()
{

}

void Service::BroadCast(std::shared_ptr<SendBuffer> _sendBuffer)
{
	WRITE_LOCK;

	for (const auto& session : sessions)
	{

		session->Send(_sendBuffer);
	}
}

std::shared_ptr<Session> Service::CreateSession()
{
	auto session = sessionFactory();
	session->SetService(shared_from_this());
	if (iocpService->RegisterSockToIOCP(session->GetSocket()) == false)
		return nullptr;

	return session;
}

void Service::AddSession(std::shared_ptr<Session> _session)
{
	WRITE_LOCK;
	sessionCount++;
	sessions.insert(_session);
}

void Service::ReleaseSession(std::shared_ptr<Session> _session)
{
	WRITE_LOCK;

	ASSERT_CRASH(sessions.erase(_session) == 0, "_session is not exist in the sessions.");
	
	sessionCount--;
}

int32 Service::GetCurrentSessionCount()
{
	return sessionCount;
}

int32 Service::GetMaxSessionCount()
{
	return maxSessionCount;
}

ServiceType Service::GetServiceType()
{
	return serviceType;
}

NetAddress Service::GetNetAddress()
{
	return netAddress;
}

std::shared_ptr<IocpService> Service::GetIocpService()
{
	return iocpService;
}

/*==============================
*		ClientService
==============================*/

ClientService::ClientService(NetAddress _targetAddress, 
	std::shared_ptr<IocpService> _iocpService, 
	SessionFactory _factory, int32 _maxSessionCount)
	:Service(ServiceType::Client,_targetAddress,_iocpService,_factory,_maxSessionCount)
{

}

bool ClientService::Start()
{
	if(CanStart()==false)
		return false;

	// 더미 테스트 시 클라 갯수 여러개일 수 있음
	const int32 sessionCount = GetMaxSessionCount();
	for (int32 i = 0; i < sessionCount; i++) {
		std::shared_ptr<Session> session = CreateSession();
		if (session->Connect() == false)
			return false;
	}

	return true;
}


/*==============================
*		ServerService
==============================*/

ServerService::ServerService(NetAddress _targetAddress, 
	std::shared_ptr<IocpService> _iocpService, SessionFactory _factory,
	int32 _maxSessionCount)
	:Service(ServiceType::Server,_targetAddress,_iocpService,_factory,
		_maxSessionCount)
{

}
//#include "ObjectPool.h"
bool ServerService::Start()
{
	if(CanStart()==false)
		return false;

	listener = ObjectPool<Listener>::MakeShared();
	if (listener == nullptr)
		return false;

	/* shared_ptr을 형변환 */
	std::shared_ptr<ServerService> serverService = 
		static_pointer_cast<ServerService>(shared_from_this());
	
	if (listener->StartAccept(serverService) == false)
		return false;

	return true;
}

void ServerService::CloseService()
{

}
