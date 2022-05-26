#pragma once

class NetAddress
{
public:
	NetAddress() = default;
	NetAddress(SOCKADDR_IN _sockAddr);
	NetAddress(string _ip, uint16 _port);
	// UTF-16 버전
	NetAddress(wstring _ip, uint16 _port);

	SOCKADDR_IN& GetSockAddr() { return sockAddr; }

	static IN_ADDR	IP2Address(const char* _ip);
	// UTF-16 버전
	static IN_ADDR	IP2AddressW(const WCHAR* _ip);
private:
	SOCKADDR_IN		sockAddr = {};
};