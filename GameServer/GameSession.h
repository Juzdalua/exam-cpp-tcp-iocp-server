#pragma once
#include "Session.h"

/*---------------
	Game Session
---------------*/
class GameSession : public Session
{
public:
	virtual ~GameSession()
	{
		cout << "~GameSession" << endl;
	}

public:
	virtual void OnConnected() override;
	virtual void OnDisconnected() override;
	virtual int32 OnRecv(BYTE* buffer, int32 len) override;
	//virtual void OnRecvPacket(BYTE* buffer, int32 len) override;
	virtual void OnSend(int32 len) override;

public:
	vector<PlayerRef> _players;
};

/*----------------------
	Game Packet Session
----------------------*/
class GamePacketSession : public PacketSession
{
public:
	virtual ~GamePacketSession()
	{
		cout << "~GameSession" << endl;
	}

public:
	virtual void OnConnected() override;
	virtual void OnDisconnected() override;
	virtual void OnRecvPacket(BYTE* buffer, int32 len) override;
	virtual void OnSend(int32 len) override;

public:
	vector<PlayerRef> _players;
};