#pragma once
#include "Session.h"
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
		DebugLog::PrintColorText(LogColor::YELLOW, "[Session Close]", "", true, false);
		if (_player != nullptr) {
			GRoom.PushJob(make_shared<LeaveRoomJob>(GRoom, _player));
			
			if (_accountId && _accountId != 0) {
				DebugLog::PrintColorText(LogColor::YELLOW, "[AccountId-> ", to_string(_accountId), false, false);
			}
			DebugLog::PrintColorText(LogColor::YELLOW, " / PlayerId-> ", to_string(_player->GetPlayerId()), false, false);
			DebugLog::PrintColorText(LogColor::YELLOW, "]", "", false, true);
		}
	}

	GameProtobufSessionRef GetProtobufSessionRef() { return static_pointer_cast<GameProtobufSession>(shared_from_this()); }

public:
	virtual void OnConnected() override;
	virtual void OnDisconnected() override;
	virtual void OnRecvPacket(BYTE* buffer, int32 len) override;
	virtual void OnSend(int32 len) override;
	virtual void OnSend(int32 len, vector<SendBufferRef>& sendVec) override;

public:
	PlayerRef _player = nullptr;
	uint64 _accountId;
};

void HandlePacketStartLog(string type, LogColor color, PacketHeader* recvHeader, GameProtobufSessionRef& session);