#include "pch.h"
#include "Session.h"
#include "SocketUtils.h"

Session::Session()
{
	_clientSocket = SocketUtils::CreateSocket();
}

Session::~Session()
{
	SocketUtils::Close(_clientSocket);
}

bool Session::ClientConnectEx()
{
	// TODO
	return false;
}

void Session::DisconnectEx(const WCHAR* cause)
{
	// TODO
}
