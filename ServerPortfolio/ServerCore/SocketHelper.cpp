#include "Core.h"
#include "SocketHelper.h"

void SocketHelper::Init()
{
	WSADATA wsaData;
	ASSERT_CRASH(::WSAStartup(MAKEWORD(2, 2), OUT & wsaData) == 0, "Failed - WSAStartup");

	SOCKET dummySocket = CreateSocket();

	ASSERT_CRASH(BindWinsockFuntion(dummySocket, WSAID_CONNECTEX,
		reinterpret_cast<LPVOID*>(&ConnectEx)),
		"Failed - Get a function pointer to ConnectEx");

	ASSERT_CRASH(BindWinsockFuntion(dummySocket, WSAID_DISCONNECTEX,
		reinterpret_cast<LPVOID*>(&DisconnectEx)),
		"Failed - Get a function pointer to DisconnectEx");

	ASSERT_CRASH(BindWinsockFuntion(dummySocket, WSAID_ACCEPTEX,
		reinterpret_cast<LPVOID*>(&AcceptEx)),
		"Failed - Get a function pointer to AcceptEx");
	CloseSocket(dummySocket);
}

SOCKET SocketHelper::CreateSocket()
{
	return WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, NULL, 0,
		WSA_FLAG_OVERLAPPED);
}

bool SocketHelper::BindWinsockFuntion(SOCKET _socket, GUID _guid, LPVOID* _funcPtr)
{
	DWORD dummyWord = 0;
	/* WSAIoctl ( 소켓의 모드를 조정하는 함수 )
	
	- SIO_GET_EXTENSION_FUNCTION_POINTER는 윈속 확장 함수 사용을 위한 포인터 요청 코드

	예)	WSAIoctl( ListenSocket, SIO_GET_EXTENSION_FUNCTION_POINTER,
		  &GuidAcceptEx, sizeof(GuidAcceptEx),
		  &lpfnAcceptEx, sizeof(lpfnAcceptEx),
		  &dwBytes, NULL, NULL);
		  	
	 윈도우 비스타 이후부터 윈속 확장 함수 포인터를 받아 사용 가능
	 (반드시 runtime에 받아야 함)
	*/

	
	return SOCKET_ERROR != ::WSAIoctl(
		_socket, SIO_GET_EXTENSION_FUNCTION_POINTER, &_guid, sizeof(_guid)
		, _funcPtr, sizeof(*_funcPtr), &dummyWord, NULL, NULL);
}

void SocketHelper::CloseSocket(SOCKET& _socket)
{
	if (INVALID_SOCKET != _socket)
		::closesocket(_socket);
	_socket = INVALID_SOCKET;
}

bool SocketHelper::SyncSocketContext(SOCKET _socket, SOCKET _listenSocket)
{
	return SetSocketOption(_socket, SOL_SOCKET, SO_UPDATE_ACCEPT_CONTEXT, _listenSocket);
}

bool SocketHelper::SetLinger(SOCKET _socket, uint16 _onOff, uint16 _linger)
{
	LINGER opt{ _onOff, _linger };
	return SetSocketOption(_socket, SOL_SOCKET, SO_LINGER, opt);
}

bool SocketHelper::SetReuseAddress(SOCKET _socket, bool _flag)
{
	return SetSocketOption(_socket, SOL_SOCKET, SO_REUSEADDR, _flag);
}

bool SocketHelper::SetRecvBufferSize(SOCKET _socket, int32 _size)
{
	return SetSocketOption(_socket, SOL_SOCKET, SO_RCVBUF, _size);
}

bool SocketHelper::SetSendBufferSize(SOCKET _socket, int32 _size)
{
	return SetSocketOption(_socket, SOL_SOCKET, SO_SNDBUF, _size);
}

bool SocketHelper::SetTcpNoDelay(SOCKET _socket, bool _flag)
{
	return SetSocketOption(_socket, SOL_SOCKET, TCP_NODELAY, _flag);
}

bool SocketHelper::Bind(SOCKET _socket, NetAddress _netAddress)
{
	return SOCKET_ERROR != ::bind(_socket,
		reinterpret_cast<const SOCKADDR*>(&_netAddress.GetSockAddr()),
		sizeof(SOCKADDR_IN));
}

bool SocketHelper::BindAnyAddress(SOCKET _socket, uint16 _port)
{
	SOCKADDR_IN myAddress;
	myAddress.sin_family = AF_INET;
	myAddress.sin_addr.s_addr = ::htonl(INADDR_ANY);
	myAddress.sin_port = _port;

	return SOCKET_ERROR != ::bind(_socket,
		reinterpret_cast<const SOCKADDR*>(&myAddress),
		sizeof(myAddress));
}

bool SocketHelper::Listen(SOCKET _socket, int32 _backlog)
{
	return SOCKET_ERROR != ::listen(_socket, _backlog);
}
