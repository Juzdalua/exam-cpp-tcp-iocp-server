#pragma once
#include "Session.h"
#include "ClientPacketHandler.h"

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
		cout << "~GameProtobufSession" << endl;
	}

	GameProtobufSessionRef GetProtobufSessionRef() { return static_pointer_cast<GameProtobufSession>(shared_from_this()); }

public:
	virtual void OnConnected() override;
	virtual void OnDisconnected() override;
	virtual void OnRecvPacket(BYTE* buffer, int32 len) override;
	virtual void OnSend(int32 len) override;

public:
	vector<PlayerRef> _players;
};

template<typename T>
void SendProtobuf(T& pkt, uint16 packetId, GameProtobufSessionRef& session) {
	const uint16 dataSize = static_cast<uint16>(pkt.ByteSizeLong());
	const uint16 packetSize = dataSize + sizeof(PacketHeader);

	SendBufferRef sendBuffer = SendBufferRef(new SendBuffer(4096));
	PacketHeader* header = reinterpret_cast<PacketHeader*>(sendBuffer->Buffer());
	header->size = packetSize;
	header->id = packetId;

	ASSERT_CRASH(pkt.SerializeToArray(&header[1], dataSize));
	sendBuffer->SetWriteSizeWithDataSize(dataSize);
	session->Send(sendBuffer);
};