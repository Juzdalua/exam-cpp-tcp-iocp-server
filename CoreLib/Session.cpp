#include "pch.h"
#include "Session.h"
#include "SocketUtils.h"

mutex m;

Session::Session()
	:buffer(BUFFER_SIZE)
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
	lock_guard<mutex> lock(m);
	for (auto& s : _sessions)
	{
		_sessions.erase(s);
	}
	_sessions.clear();
}
