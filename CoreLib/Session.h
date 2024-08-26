#pragma once
#include "IocpCore.h"
#include "IocpEvent.h"

/*
	Connected Client Infomation
*/
class Session : public IocpObject
{
	friend IocpCore;

	enum
	{
		BUFFER_SIZE = 0x10000,
	};

public:
	Session();
	virtual ~Session();

public:
	SOCKET GetSocket() { return _clientSocket; }
	bool IsConnected() { return _connected; }

	bool ClientConnectEx();
	void DisconnectEx(const WCHAR* cause);

private:
	SOCKET _clientSocket = INVALID_SOCKET;
	atomic<bool> _connected = false;

	char _recvBuffer[100];
	queue<vector<char[100]>> sendQueue;

private:
	/* IocpEvent Àç»ç¿ë */
	ConnectEvent _connectEvent;
	DisconnectEvent _disconnectEvent;
	RecvEvent _recvEvent;
	SendEvent _sendEvent;
};

