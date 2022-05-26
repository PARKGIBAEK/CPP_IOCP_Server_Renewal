module;
#include <WinSock2.h>
#include <MSWSock.h>
#include <vector>
export module Listener;
import Types;
import NetAddress;
import SocketUtils;

export class Listener
{
public:
	Listener() = default;
	~Listener()
	{
		SocketUtils::Close(socket);
	}

	bool	StartAccept()
	{
		socket = SocketUtils::CreateSocket();
		if (socket == INVALID_SOCKET)
			return false;

		if (SocketUtils::SetReuseAddress(socket, true) == false)
			return false;

		if (SocketUtils::SetLinger(socket, 0, 0) == false)
			return false;

		if(SocketUtils::Bind(socket,netAddress)==false)
			return false;

		if (SocketUtils::Listen(socket) == false)
			return false;

		const int32 acceptCount = SOMAXCONN;
		for (size_t i = 0; i < acceptCount; i++)
		{
			
		}
	}
private:
	SOCKET socket = INVALID_SOCKET;
	NetAddress netAddress;
};