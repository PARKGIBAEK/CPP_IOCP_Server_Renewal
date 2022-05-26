#pragma once
#include "IocpService.h"

class Listener : public ISession
{
public:
	Listener() = default;
	~Listener();

	// Inherited via ISession
	virtual SOCKET GetSocket() override;
	virtual void DispatchEvent(IocpEvent* _iocpEvent, int32 _byteTransferred = 0) override;

	// Use externally
	bool StartAccept(std::shared_ptr<class ServerService> _service);
	void CloseSocket();

private:
	// �񵿱� �����Լ� AcceptEx �ɱ�
	void RegisterAccept(class AcceptEvent* _acceptEvent);
	void ProcessAccept(class AcceptEvent* _acceptEvent);

protected:
	SOCKET listenSocket = INVALID_SOCKET;
	Vector<class AcceptEvent*> acceptEvents;
	std::shared_ptr<class ServerService> ownerService;
};