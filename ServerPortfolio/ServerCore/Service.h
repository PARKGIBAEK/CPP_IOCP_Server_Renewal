#pragma once
//#include "Listener.h"
enum class ServiceType :uint8
{
	Server,
	Client,
};


using SessionFactory = std::function<std::shared_ptr<class Session>(void)>;

class Service :public std::enable_shared_from_this<Service>
{
public:

	Service(ServiceType _type, NetAddress _address,
		std::shared_ptr<class IocpService> _iocpService, SessionFactory _factory,
		int32 _maxSessionCount = 1);

	virtual ~Service();

	virtual bool					Start() = 0;
	bool							CanStart();

	virtual void					CloseService();
	void							SetSessionFactory(SessionFactory _factory)
	{
		sessionFactory = _factory;
	}

	void							BroadCast(std::shared_ptr<SendBuffer> _sendBuffer);
	
	[[nodiscard]]
	std::shared_ptr<Session>		CreateSession();
	void							AddSession(std::shared_ptr<Session> _session);

	void							ReleaseSession(std::shared_ptr<Session> _session);
	int32							GetCurrentSessionCount();
	int32							GetMaxSessionCount();

	ServiceType						GetServiceType();
	NetAddress						GetNetAddress();
	std::shared_ptr<IocpService>	GetIocpService();

protected:
	USE_LOCK;
	ServiceType						serviceType;
	NetAddress						netAddress = {};
	std::shared_ptr<IocpService>	iocpService;

	Set<std::shared_ptr<Session>>	sessions;
	int32							sessionCount = 0;
	int32							maxSessionCount = 0; // Server쪽일 경우에는 listener 동접자 수
	SessionFactory					sessionFactory;
};


class ClientService :public Service
{
public:
	ClientService(NetAddress _targetAddress, std::shared_ptr<class IocpService> _iocpService,
		SessionFactory _factory, int32 _maxSessionCount = 1);
	
	virtual ~ClientService() {}
	
	virtual bool Start() override;
};

class ServerService :public Service
{
public:
	ServerService(NetAddress _targetAddress, std::shared_ptr<class IocpService> _iocpService,
		SessionFactory _factory, int32 _maxSessionCount = 1);
	
	virtual ~ServerService() {}
	
	virtual bool Start() override;
	
	virtual void CloseService() override;

private:
	std::shared_ptr<class Listener> listener;
};