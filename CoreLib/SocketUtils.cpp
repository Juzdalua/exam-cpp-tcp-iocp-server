#include "pch.h"
#include "SocketUtils.h"
#include "Session.h"

void SocketUtils::Init()
{
	WSADATA wsaData;
	ASSERT_CRASH(WSAStartup(MAKEWORD(2, 2), OUT &wsaData) == 0);
}

void SocketUtils::Clear()
{
	WSACleanup();
}

SOCKET SocketUtils::CreateSocket()
{
	return WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, NULL, 0, WSA_FLAG_OVERLAPPED);
}

bool SocketUtils::Bind(SOCKET socket, wstring ip, uint16 port)
{
	return false;
}

bool SocketUtils::BindAnyAddress(SOCKET listenSocket, uint16 port)
{
	SOCKADDR_IN myAddress;
	myAddress.sin_family = AF_INET;
	myAddress.sin_addr.s_addr = htonl(INADDR_ANY);
	myAddress.sin_port = htons(port);

	return SOCKET_ERROR != ::bind(listenSocket, reinterpret_cast<const SOCKADDR*>(&myAddress), sizeof(myAddress));
}

bool SocketUtils::Listen(SOCKET listenSocket, int32 backlog)
{
	return SOCKET_ERROR != listen(listenSocket, backlog);
}

void SocketUtils::Accept()
{
	for (int32 i = 0; i < 5; i++)
	{
		shared_ptr<Session> sessionRef = make_shared<Session>();
		GSessionManager._sessionManager.push_back(sessionRef);
	}
}


void SocketUtils::Close(SOCKET& socket)
{
	if (socket != INVALID_SOCKET)
		closesocket(socket);
	socket = INVALID_SOCKET;
}
