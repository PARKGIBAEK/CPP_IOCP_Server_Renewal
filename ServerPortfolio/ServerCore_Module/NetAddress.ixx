module;
//#define WIN32_LEAN_AND_MEAN
#include <WinSock2.h>
#include <ws2tcpip.h>
#include <string>

export module NetAddress;
import Types;

export class NetAddress
{
public:
	NetAddress(SOCKADDR_IN _sockAddr) :sockAddr(_sockAddr)
	{	}
	NetAddress(std::wstring _ip, uint16 _port)
	{
		::memset(&sockAddr, 0, sizeof(sockAddr));
		sockAddr.sin_family = AF_INET;
		sockAddr.sin_addr = IpToAddress(_ip);
		sockAddr.sin_port = ::htons(_port);
	}

	static IN_ADDR	IpToAddress(std::wstring _ip)
	{
		IN_ADDR address;
		/* ::InetPtonW �Լ���
		  Ÿ���� ǥ�� �ؽ�Ʈ ���� IP�� numeric binary�� �ٲ۴�.(wide string����)
		  ANSI ������ 

		*/
		::InetPtonW(AF_INET, _ip.c_str(), &address);

		return address;
	}

	SOCKADDR_IN& GetSockAddr() 
	{
		return  sockAddr;
	}

	std::wstring		GetIpAddress()
	{
		WCHAR buffer[100];
		::InetNtopW(AF_INET, &sockAddr.sin_addr, buffer, Len32(buffer));

		return std::wstring(buffer);
	}
private:
	SOCKADDR_IN sockAddr;
};