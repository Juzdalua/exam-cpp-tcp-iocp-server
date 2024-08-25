#pragma once

/*
	Connected Client Infomation
*/
class Session
{
	enum
	{
		BUFFER_SIZE = 0x10000,
	};

public:
	Session();
	virtual ~Session();

public:

private:
	SOCKET _clientSocket = INVALID_SOCKET;
};

