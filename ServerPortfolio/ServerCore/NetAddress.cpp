#include "Core.h"
#include "NetAddress.h"

NetAddress::NetAddress(SOCKADDR_IN _sockAddr):sockAddr(_sockAddr)
{	
}

NetAddress::NetAddress(string _ip, uint16 _port)
{
	memset(&sockAddr, 0, sizeof(sockAddr));
	sockAddr.sin_family = AF_INET;
	sockAddr.sin_addr = IP2Address(_ip.c_str());
	sockAddr.sin_port = _port;
}

NetAddress::NetAddress(wstring _ip, uint16 _port)
{
	memset(&sockAddr, 0, sizeof(sockAddr));
	sockAddr.sin_family = AF_INET;
	sockAddr.sin_addr= IP2AddressW(_ip.c_str());
	sockAddr.sin_port = _port;
}

IN_ADDR NetAddress::IP2Address(const char* _ip)
{
	IN_ADDR address;
	inet_pton(AF_INET, _ip, &address);
	return IN_ADDR();
}

IN_ADDR NetAddress::IP2AddressW(const WCHAR* _ip)
{
	IN_ADDR address;
	::InetPtonW(AF_INET, _ip, &address);

	return address;
}
