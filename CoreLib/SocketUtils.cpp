#include "pch.h"
#include "SocketUtils.h"

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

bool SocketUtils::AcceptEx(SOCKET listenSocket, SOCKET clientSocket, PVOID outputBuffer, DWORD recvDataLen, DWORD localAddressLen, DWORD remoteAddressLen, DWORD bytesReceived, LPOVERLAPPED iocpEvent)
{
	// TODO
	return false;
}


void SocketUtils::Close(SOCKET& socket)
{
	if (socket != INVALID_SOCKET)
		closesocket(socket);
	socket = INVALID_SOCKET;
}