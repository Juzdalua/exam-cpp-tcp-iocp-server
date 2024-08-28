#pragma once
#include "NetAddress.h"
#include "IocpEvent.h"

class Session
{
	enum
	{
		BUFFER_SIZE = 0x10000, // 64KB
	};

public:
	Session();
	~Session();

public:
	SOCKET GetSocket() { return _clientSocket; }
	bool IsConnected() { return _connected; }

	void SetNetAddress(NetAddress address) { _netAddress = address; }
	NetAddress GetAddress() { return _netAddress; }

	// TEMP
	vector<BYTE> _buffer;
	atomic<bool> _connected = false;
	NetAddress _netAddress = {};

private:
	SOCKET _clientSocket = INVALID_SOCKET;
	//vector<BYTE> _buffer;
	//atomic<bool> _connected = false;
	//NetAddress _netAddress = {};

public: // TODO-> private
	/* IocpEvent Àç»ç¿ë */
	ConnectEvent _connectEvent;
	DisconnectEvent _disconnectEvent;
	RecvEvent _recvEvent;
	SendEvent _sendEvent;
};

class SessionManager
{
public:
	SessionManager();
	~SessionManager();

public:
	set<SessionRef> _sessions;
};