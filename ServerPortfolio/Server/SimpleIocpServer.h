#pragma once

enum class EventType : uint8
{
	Connect,
	Disconnect,
	Accept,
	Recv,
	Send
};

class IOCP_Event :public OVERLAPPED
{
public:
	IOCP_Event(EventType _type) :eventType(_type)
	{
		//memset((void*)this, 0, sizeof(OVERLAPPED));
	}

	void InitOverlapped()
	{
		OVERLAPPED::hEvent = 0;
		OVERLAPPED::Internal = 0;
		OVERLAPPED::InternalHigh = 0;
		OVERLAPPED::Offset = 0;
		OVERLAPPED::OffsetHigh = 0;
		//OVERLAPPED::Pointer = 0;
	}

	EventType eventType;
};

class AcceptEvent :public IOCP_Event
{
public:
	AcceptEvent() = delete;
	AcceptEvent(const AcceptEvent&) = delete;
	AcceptEvent(std::shared_ptr<class MyListener> _owner) :IOCP_Event(EventType::Accept), owner(_owner)
	{	}


public:
	std::shared_ptr<class MyListener> owner = nullptr;
};

class MyListener :public std::enable_shared_from_this<MyListener>
{
public:
	MyListener() = delete;
	MyListener(class IOCP_Server* _owner) :owner(_owner)
	{
	}
	~MyListener() {	}

	bool StartListening(uint32 _backlog)
	{
		if (SOCKET_ERROR == ::WSAStartup(MAKEWORD(2, 2), &wsaData))
		{
			//error handling
			return false;
		}

		listenSocket = owner->CreateSocket();
		if (INVALID_SOCKET != owner->RegisterSock2IOCP(listenSocket))
			return false;

		int optVal = true;

		// SetReuseAddress
		if (false == SetSockOpt<bool>(listenSocket, SOL_SOCKET, SO_REUSEADDR, true))
		{
			//error handling
			return false;
		}

		// SetLinger
		if (false == SetSockOpt(listenSocket, SOL_SOCKET, SO_LINGER, LINGER{ 0,0 }))
		{
			//error handling
			return false;
		}

		// Bind
		memset(&sockAddr, 0, sizeof(sockAddr));
		sockAddr.sin_family = AF_INET;
		sockAddr.sin_addr.s_addr = ::htonl(INADDR_ANY);
		sockAddr.sin_port = ::htons(7777);
		if (SOCKET_ERROR != ::bind(listenSocket, (const sockaddr*)&sockAddr, sizeof(sockAddr)))
		{
			//error handling
			return false;
		}

		// Listen
		if (SOCKET_ERROR != ::listen(listenSocket, _backlog))
		{
			//error handling
			return false;
		}

		// 일반 accept
		while (true)
		{
			SOCKADDR_IN sockAddress;
			SOCKET clientSocket = accept(listenSocket, (sockaddr*)&sockAddress, (int*)sizeof(sockAddress));
			char buffer[100] = { 0, };
			::InetNtop(AF_INET, &sockAddress, (PWSTR)buffer, sizeof(buffer));
			printf("%s 에서 접속했습니다.\n", ::inet_ntoa(sockAddress.sin_addr));
		}

		// AcceptEx 사용 시
		GUID guid = WSAID_ACCEPTEX;
		::WSAIoctl(listenSocket, SIO_GET_EXTENSION_FUNCTION_POINTER, 
			&guid, sizeof(guid), AcceptEx, sizeof(AcceptEx), 0, 0, 0);
		for (size_t i = 0; i < _backlog; i++)
		{
			AcceptEvent* acceptEvent = new AcceptEvent(shared_from_this());
			acceptEvents.emplace_back(acceptEvent);
			RegisterAccept(acceptEvent);
		}
	}

	void RegisterAccept(AcceptEvent* _acceptEvent)
	{
		_acceptEvent->InitOverlapped();
		_acceptEvent->owner = shared_from_this();

		DWORD bytesTransferred = 0;
		if(false==AcceptEx(listenSocket,owner->CreateSocket(),))
	}
	bool SyncSockContext(SOCKET _socket, SOCKET _listenSocket)
	{
		return SetSockOpt(_socket, SOL_SOCKET, SO_UPDATE_ACCEPT_CONTEXT, _listenSocket);
	}

	template<typename T>
	static inline bool SetSockOpt(SOCKET _socket, int32 level, int32 _optName, T _optVal)
	{
		return SOCKET_ERROR != ::setsockopt(_socket, level, _optName, reinterpret_cast<const char*>(&_optVal), sizeof(T));
	}


private:
	WSAData						wsaData;
	SOCKET						listenSocket = INVALID_SOCKET;
	SOCKADDR_IN					sockAddr;
	IOCP_Server*				owner;
	std::vector<AcceptEvent*>	acceptEvents;
	LPFN_ACCEPTEX				AcceptEx = nullptr;
};


class IOCP_Server
{
public:
	static constexpr unsigned int BACKLOG = 1000;
	static constexpr unsigned int NUMBER_OF_WORKER_THREAD = 1000;

	IOCP_Server()
	{
		iocpHandle = ::CreateIoCompletionPort(INVALID_HANDLE_VALUE, 0, 0, 0);
		ASSERT_CRASH(iocpHandle == INVALID_HANDLE_VALUE, "Failed - CreateIoCompletionPort");
	}
	~IOCP_Server()
	{
		::CloseHandle(iocpHandle);
	}
	// Create a socket for Overlapped I/O
	SOCKET CreateSocket()
	{
		SOCKET socket = ::WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, NULL, 0, WSA_FLAG_OVERLAPPED);
		return RegisterSock2IOCP(socket);
	}

	SOCKET RegisterSock2IOCP(SOCKET _socket)
	{
		ASSERT_CRASH(NULL != ::CreateIoCompletionPort(reinterpret_cast<HANDLE>(_socket), iocpHandle, 0, 0),"Failed - Register a socket to IOCP");

		return _socket;
	}

	bool Run()
	{
		threads.emplace_back(std::thread([=]() {
			StartIocpServer();
			}));
		for (size_t i = 0; i < NUMBER_OF_WORKER_THREAD; i++)
		{
			threads.emplace_back(std::thread([=]() {
				WorkerThread();
				}));
		}

		for (auto& thread : threads)
			if (thread.joinable())
				thread.join();
	}

	void WorkerThread()
	{
		while (true)
		{
			DispatchIoCompletionPacket();
		}
	}

	bool DispatchIoCompletionPacket()
	{
		DWORD bytesTransferred = 0;
		ULONG_PTR key = 0;
		OVERLAPPED overlapped;
		IOCP_Event* iocpEvent = nullptr;
		if (::GetQueuedCompletionStatus(iocpHandle, &bytesTransferred, &key, (LPOVERLAPPED*)iocpEvent, INFINITE))
		{
			auto eventType = iocpEvent->eventType;
			ProcessIocpEvent(eventType, bytesTransferred);
		}
		else
		{
			auto errorCode = ::WSAGetLastError();
			if (errorCode == WAIT_TIMEOUT)
				return false;
		}

		return  true;
	}

	void ProcessIocpEvent(EventType _event, int32 _bytesTransferred)
	{
		switch (_event)
		{
		case EventType::Connect:
			ProcessConnect();
			break;
		case EventType::Disconnect:
			ProcessDisconnect();
			break;
		case EventType::Recv:
			ProcessRecv();
			break;
		case EventType::Send:
			ProcessSend();
			break;
		default:
			break;

		}
	}


	bool StartIocpServer()
	{
		listener.StartListening(BACKLOG);
	}


private:
	HANDLE iocpHandle;
	MyListener listener = MyListener(this);
	std::vector<std::thread> threads;
};