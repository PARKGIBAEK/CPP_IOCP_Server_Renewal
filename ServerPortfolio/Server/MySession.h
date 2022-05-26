#pragma once

class MySession
{
	constexpr static unsigned int BUFFER_SIZE = 0x10000; // 64 KB
public:
	MySession();
	virtual ~MySession();

private:
	SOCKET socket;
	NetAddress netAddress;
	bool isConnected;
	unsigned char recvBuffer[BUFFER_SIZE] = { 0, };
	unsigned char sendBuffer[BUFFER_SIZE] = { 0, };
};

