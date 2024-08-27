#pragma once
class Session
{
public:
	Session();
	~Session();

public:
	SOCKET GetSocket() { return _clientSocket; }

	// TEMP
	vector<BYTE> buffer;

private:
	SOCKET _clientSocket = INVALID_SOCKET;
};
