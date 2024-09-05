#pragma once
#include "Session.h"
#include "ClientPacketHandler.h"
#include "Room.h"

/*---------------
	Game Session
	Without PakcetHeader
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
	With PakcetHeader
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

/*-------------------
	Game Protobuf Session
	With PakcetHeader
-------------------*/
class GameProtobufSession : public PacketSession
{
public:
	virtual ~GameProtobufSession()
	{
		if (_player != nullptr) {
			GRoom.Leave(_player);
		}
		cout << "~GameProtobufSession" << endl;
	}

	GameProtobufSessionRef GetProtobufSessionRef() { return static_pointer_cast<GameProtobufSession>(shared_from_this()); }

public:
	virtual void OnConnected() override;
	virtual void OnDisconnected() override;
	virtual void OnRecvPacket(BYTE* buffer, int32 len) override;
	virtual void OnSend(int32 len) override;

public:
	PlayerRef _player;
};
