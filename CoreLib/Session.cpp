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

SessionManager::SessionManager()
{
}

SessionManager::~SessionManager()
{
	for (Session* session : _sessionManager)
	{
		delete session;
	}
	_sessionManager.clear();
}
