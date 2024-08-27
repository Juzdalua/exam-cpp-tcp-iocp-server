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

	// TEMP
	char* GetRecvBuffer() { return _recvBuffer; }

private:
	virtual HANDLE GetHandle() override;
	virtual void Dispatch(class IocpEvent* iocpEvent, int32 numOfBytes = 0) override;

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

class SessionManager
{
public:
	SessionManager() {};
	virtual ~SessionManager() {};

public:
	vector<shared_ptr<Session>> _sessionManager = {};
};

extern SessionManager GSessionManager;