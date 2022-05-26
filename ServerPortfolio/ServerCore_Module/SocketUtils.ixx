module;
#include "CoreMacros.h"
#include <WinSock2.h>
#include <MSWSock.h>
export module SocketUtils;
import Types;
import NetAddress;

/*
	The function pointer for the ConnectEx function must be obtained
	at run time by making a call to the WSAIoctl function with the
	SIO_GET_EXTENSION_FUNCTION_POINTER opcode specified.

	The input buffer passed to the WSAIoctl function must contain WSAID_CONNECTEX,
	a globally unique identifier (GUID) whose value identifies
	the ConnectEx extension function.

	On success, the output returned by the WSAIoctl function contains a pointer to the
	ConnectEx function.

	The WSAID_CONNECTEX GUID is defined in the Mswsock.h header file.

	자세한 내용은 MSDN 참고
*/

template<typename T>
static inline bool SetSockOpt(SOCKET _socket, int32 _level, int32 _optName, T _optVal)
{
	return SOCKET_ERROR != ::setsockopt(_socket, _level, _optName, reinterpret_cast<const char*>(&_optVal), sizeof(T));
}

export class SocketUtils
{
public:
	static LPFN_CONNECTEX			ConnectEx;
	static LPFN_DISCONNECTEX		DisconnectEx;
	static LPFN_ACCEPTEX			AcceptEx;

public:
	static void Init()
	{
		WSADATA wsaData;
		ASSERT(::WSAStartup(MAKEWORD(2, 2), &wsaData) == 0, "WSAStartup failed");

		/* 런타임 주소 얻어오기*/
		SOCKET dummySocket = CreateSocket();

		/* Winsock 확장 함수 포인터를 런타임에 받아오기 (런타임에만 받아 올 수 있음)*/
		ASSERT(BindWinsockFunc(dummySocket, WSAID_CONNECTEX, reinterpret_cast<LPVOID*>(&ConnectEx)), "Failed to get CONNECTEX FuncPtr");
		ASSERT(BindWinsockFunc(dummySocket, WSAID_DISCONNECTEX, reinterpret_cast<LPVOID*>(&DisconnectEx)), "Failed to get CONNECTEX FuncPtr");
		ASSERT(BindWinsockFunc(dummySocket, WSAID_ACCEPTEX, reinterpret_cast<LPVOID*>(&AcceptEx)), "Failed to get CONNECTEX FuncPtr");
	}

	static void Clear()
	{
		::WSACleanup();
	}

	static SOCKET CreateSocket()
	{
		/* WSA_FLAG_OVERLAPPED 옵션
		  : Overlapped IO 소켓을 생성하며,
			해당 소켓은 WSASend, WSASendTo, WSARecv, WSARecvFrom, WSAIoctl등의 함수 사용 가능.
			이 옵션을 주지 않으면 Non-Overlapped 소켓으로 만들어진다. */
		return ::WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, NULL, 0, WSA_FLAG_OVERLAPPED);
	}

	static bool BindWinsockFunc(SOCKET _socket, GUID _guid, LPVOID* _fn)
	{
		/* WSAIoctl ( 소켓의 모드를 조정하는 함수 )

		  사용 예시)
		  WSAIoctl( ListenSocket, SIO_GET_EXTENSION_FUNCTION_POINTER,
		  &GuidAcceptEx, sizeof (GuidAcceptEx),
		  &lpfnAcceptEx, sizeof (lpfnAcceptEx),
		  &dwBytes, NULL, NULL);

		 [in] s : socket
		 [in] dwIoControlCode : The control code of operation to perform.
		 [in] lpvInBuffer : A pointer to the input buffer (winsock확장 함수 포인터를 요청할 경우 GUID)
		 [in] cbInBuffer : The size, in bytes, of the input buffer.
		 [out] lpvOutBuffer : A pointer to the output buffer.
		 [in] cbOutBuffer : The size, in bytes, of the output buffer.
		 [out] lpcbBytesReturned : A pointer to actual number of bytes of output.
		 [in] lpOverlapped : A pointer to a WSAOVERLAPPED structure (ignored for non-overlapped sockets).
		 [in] lpCompletionRoutine : Type - _In_opt_ LPWSAOVERLAPPED_COMPLETION_ROUTINE

		*/

		// SIO_GET_EXTENSION_FUNCTION_POINTER는 윈속 확장 함수 사용을 위한 포인터 요청 코드
		DWORD bytes = 0;
		return SOCKET_ERROR != ::WSAIoctl(_socket, SIO_GET_EXTENSION_FUNCTION_POINTER, &_guid, sizeof(_guid),
			_fn, sizeof(*_fn), &bytes, NULL, NULL);
	}

	static bool SetLinger(SOCKET _socket, uint16 _onOff, uint16 _lingerTime)
	{ // LINGER 옵션은 소켓을 close했을 때 전송되지 않은 데이터를 어떻게 처리할 것인가에 대한 옵션
		LINGER option{ _onOff,_lingerTime };

		return SetSockOpt(_socket, SOL_SOCKET, SO_LINGER, option);
	}

	static bool SetReuseAddress(SOCKET _socket, bool _flag)
	{ /* 커널이 소켓의 포트를 점유하고 있는 상황에서 서버 프로그램이 종료된 후에도
		커널은 일정 시간 동안  소켓을 계속 점유한다.
		하지만 SO_REUSEADDR옵션을 적용하면 커널이 점유 중인 소켓을 재사용 할 수 있다.*/
		return SetSockOpt(_socket, SOL_SOCKET, SO_REUSEADDR, _flag);
	}

	static bool SetRecvBufferSize(SOCKET _socket, int32 _size)
	{ // SORCVBUF 옵션은 커널의 송신 버퍼 크기를 조절하는 옵션
		return SetSockOpt(_socket, SOL_SOCKET, SO_RCVBUF, _size);
	}

	static bool SetSendBufferSize(SOCKET _socket, int32 _size)
	{ // SORCVBUF 옵션은 커널의 송신 버퍼 크기를 조절하는 옵션
		return SetSockOpt(_socket, SOL_SOCKET, SO_SNDBUF, _size);
	}

	static bool SetTcpNoDelay(SOCKET _socket, bool _flag)
	{ // NAGLE 알고리즘 on/off
		return SetSockOpt(_socket, SOL_SOCKET, TCP_NODELAY, _flag);
	}

	static bool SetUpdateAcceptSocket(SOCKET _socket, SOCKET _listenSocket)
	{ /* ListenSocket의 특성을 Client Socket에 그대로 적용하기
	  (이걸 안하면 local / remote address 받아오는 과정 복잡해 짐)*/
		return SetSockOpt(_socket, SOL_SOCKET, SO_UPDATE_ACCEPT_CONTEXT, _listenSocket);
	}

	static bool Bind(SOCKET _socket, NetAddress _netAddr)
	{
		return SOCKET_ERROR != ::bind(_socket, reinterpret_cast<const SOCKADDR*>(&_netAddr.GetSockAddr()), sizeof(SOCKADDR_IN));
	}

	static bool BindAnyAddress(SOCKET _socket, uint16 _port)
	{
		SOCKADDR_IN address;
		address.sin_family = AF_INET;
		address.sin_addr.s_addr = ::htonl(INADDR_ANY);
		address.sin_port = ::htons(_port);

		return SOCKET_ERROR != ::bind(_socket, reinterpret_cast<const SOCKADDR*>(&address), sizeof(address));
	}

	static bool Listen(SOCKET _socket, int32 _backlog = SOMAXCONN)
	{
		return SOCKET_ERROR != ::listen(_socket, _backlog);
	}

	static void Close(SOCKET& _socket)
	{
		if (_socket != INVALID_SOCKET)
			::closesocket(_socket);
		_socket = INVALID_SOCKET;
	}
};
