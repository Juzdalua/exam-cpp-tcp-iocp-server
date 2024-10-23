#pragma once
#include "Session.h"
#include "Room.h"
#include "ThreadQueue.h"

/*-------------------
	Game Protobuf Session
	With PakcetHeader
-------------------*/
class GameProtobufSession : public PacketSession
{
public:
	virtual ~GameProtobufSession()
	{
		if (_player != nullptr)
		{
			// ΖΔΖΌ Ε»Επ
			GRoom.WithdrawParty(_player->GetPartyId(), _player);
			_player->SetPartyId(0);

			// °ΤΐΣ Ήζ Ά°³ͺ±β
			GRoom.Leave(_player);
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