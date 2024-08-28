#pragma once
#include "NetAddress.h"

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
};

class SessionManager
{
public:
	SessionManager();
	~SessionManager();

public:
	set<SessionRef> _sessions;
};