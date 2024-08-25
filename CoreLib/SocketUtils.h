#pragma once

class SocketUtils
{
public:
	static void Init();
	static void Clear();

	static SOCKET CreateSocket();
	static bool BindAnyAddress(SOCKET socket, uint16 port);
	static bool Listen(SOCKET socket, int32 backlog = SOMAXCONN);
	static void Close(SOCKET& socket);
};

