#pragma once

class SocketUtils
{
public:
	static void Init();
	static void Clear();

	static SOCKET CreateSocket();
	static bool Bind(SOCKET socket, wstring ip, uint16 port);
	static bool BindAnyAddress(SOCKET socket, uint16 port);
	static bool Listen(SOCKET socket, int32 backlog = SOMAXCONN);
	static bool AcceptEx(SOCKET listenSocket, SOCKET clientSocket, PVOID outputBuffer, DWORD recvDataLen, DWORD localAddressLen, DWORD remoteAddressLen, DWORD bytesReceived, LPOVERLAPPED iocpEvent);
	static void Close(SOCKET& socket);
};

