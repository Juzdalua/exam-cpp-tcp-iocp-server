#include "pch.h"
#include "Session.h"
#include "SocketUtils.h"

SessionManager GSessionManager;

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

HANDLE Session::GetHandle()
{
	return reinterpret_cast<HANDLE>(_clientSocket);
}

void Session::Dispatch(IocpEvent* iocpEvent, int32 numOfBytes)
{
}
