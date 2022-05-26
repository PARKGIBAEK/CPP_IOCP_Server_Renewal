#include "pch.h"
#include "MySession.h"
#include "NetAddress.h"

MySession::MySession() :socket(INVALID_SOCKET), isConnected(false)
{

}

MySession::~MySession()
{

}
