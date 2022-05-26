#pragma once
#include "NetAddress.h"
class SocketHelper
{
public:
	static LPFN_CONNECTEX			ConnectEx;
	static LPFN_DISCONNECTEX		DisconnectEx;
	static LPFN_ACCEPTEX			AcceptEx;

public:
	static void		Init();
	static SOCKET	CreateSocket();
	static bool		BindWinsockFuntion(SOCKET _socket, GUID _guid, LPVOID* _funcPtr);
	static void		CloseSocket(SOCKET& _socket);
	
	template<typename T>
	static bool		SetSocketOption(
						SOCKET _socket, int32 _level, int32 _optName, T _optVal);

	static bool		SyncSocketContext(SOCKET _socket, SOCKET _listenSocket);

	static bool		SetLinger(SOCKET _socket, uint16 _onOff, uint16 _linger);

	static bool		SetReuseAddress(SOCKET _socket, bool _flag);
	static bool		SetRecvBufferSize(SOCKET _socket, int32 _size);
	static bool		SetSendBufferSize(SOCKET _socket, int32 _size);
	static bool		SetTcpNoDelay(SOCKET _socket, bool _flag);

	static bool		Bind(SOCKET _socket, NetAddress _netAddress);
	static bool		BindAnyAddress(SOCKET _socket, uint16 _port);
	static bool		Listen(SOCKET _socket, int32 _backlog = SOMAXCONN);
	
};


template<typename T>
bool SocketHelper::SetSocketOption(SOCKET _socket, int32 _level, int32 _optName, T _optVal)
{
	return SOCKET_ERROR != ::setsockopt(_socket, _level, _optName, reinterpret_cast<const char*>(&_optVal), sizeof(T));
}
