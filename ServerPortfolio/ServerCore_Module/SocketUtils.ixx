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

	�ڼ��� ������ MSDN ����
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

		/* ��Ÿ�� �ּ� ������*/
		SOCKET dummySocket = CreateSocket();

		/* Winsock Ȯ�� �Լ� �����͸� ��Ÿ�ӿ� �޾ƿ��� (��Ÿ�ӿ��� �޾� �� �� ����)*/
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
		/* WSA_FLAG_OVERLAPPED �ɼ�
		  : Overlapped IO ������ �����ϸ�,
			�ش� ������ WSASend, WSASendTo, WSARecv, WSARecvFrom, WSAIoctl���� �Լ� ��� ����.
			�� �ɼ��� ���� ������ Non-Overlapped �������� ���������. */
		return ::WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, NULL, 0, WSA_FLAG_OVERLAPPED);
	}

	static bool BindWinsockFunc(SOCKET _socket, GUID _guid, LPVOID* _fn)
	{
		/* WSAIoctl ( ������ ��带 �����ϴ� �Լ� )

		  ��� ����)
		  WSAIoctl( ListenSocket, SIO_GET_EXTENSION_FUNCTION_POINTER,
		  &GuidAcceptEx, sizeof (GuidAcceptEx),
		  &lpfnAcceptEx, sizeof (lpfnAcceptEx),
		  &dwBytes, NULL, NULL);

		 [in] s : socket
		 [in] dwIoControlCode : The control code of operation to perform.
		 [in] lpvInBuffer : A pointer to the input buffer (winsockȮ�� �Լ� �����͸� ��û�� ��� GUID)
		 [in] cbInBuffer : The size, in bytes, of the input buffer.
		 [out] lpvOutBuffer : A pointer to the output buffer.
		 [in] cbOutBuffer : The size, in bytes, of the output buffer.
		 [out] lpcbBytesReturned : A pointer to actual number of bytes of output.
		 [in] lpOverlapped : A pointer to a WSAOVERLAPPED structure (ignored for non-overlapped sockets).
		 [in] lpCompletionRoutine : Type - _In_opt_ LPWSAOVERLAPPED_COMPLETION_ROUTINE

		*/

		// SIO_GET_EXTENSION_FUNCTION_POINTER�� ���� Ȯ�� �Լ� ����� ���� ������ ��û �ڵ�
		DWORD bytes = 0;
		return SOCKET_ERROR != ::WSAIoctl(_socket, SIO_GET_EXTENSION_FUNCTION_POINTER, &_guid, sizeof(_guid),
			_fn, sizeof(*_fn), &bytes, NULL, NULL);
	}

	static bool SetLinger(SOCKET _socket, uint16 _onOff, uint16 _lingerTime)
	{ // LINGER �ɼ��� ������ close���� �� ���۵��� ���� �����͸� ��� ó���� ���ΰ��� ���� �ɼ�
		LINGER option{ _onOff,_lingerTime };

		return SetSockOpt(_socket, SOL_SOCKET, SO_LINGER, option);
	}

	static bool SetReuseAddress(SOCKET _socket, bool _flag)
	{ /* Ŀ���� ������ ��Ʈ�� �����ϰ� �ִ� ��Ȳ���� ���� ���α׷��� ����� �Ŀ���
		Ŀ���� ���� �ð� ����  ������ ��� �����Ѵ�.
		������ SO_REUSEADDR�ɼ��� �����ϸ� Ŀ���� ���� ���� ������ ���� �� �� �ִ�.*/
		return SetSockOpt(_socket, SOL_SOCKET, SO_REUSEADDR, _flag);
	}

	static bool SetRecvBufferSize(SOCKET _socket, int32 _size)
	{ // SORCVBUF �ɼ��� Ŀ���� �۽� ���� ũ�⸦ �����ϴ� �ɼ�
		return SetSockOpt(_socket, SOL_SOCKET, SO_RCVBUF, _size);
	}

	static bool SetSendBufferSize(SOCKET _socket, int32 _size)
	{ // SORCVBUF �ɼ��� Ŀ���� �۽� ���� ũ�⸦ �����ϴ� �ɼ�
		return SetSockOpt(_socket, SOL_SOCKET, SO_SNDBUF, _size);
	}

	static bool SetTcpNoDelay(SOCKET _socket, bool _flag)
	{ // NAGLE �˰��� on/off
		return SetSockOpt(_socket, SOL_SOCKET, TCP_NODELAY, _flag);
	}

	static bool SetUpdateAcceptSocket(SOCKET _socket, SOCKET _listenSocket)
	{ /* ListenSocket�� Ư���� Client Socket�� �״�� �����ϱ�
	  (�̰� ���ϸ� local / remote address �޾ƿ��� ���� ������ ��)*/
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
