#include "pch.h"
#include "GameSession.h"
#include "GameSessionManager.h"
#include "Protocol.pb.h"

/*---------------
	Game Session
	Without Header
---------------*/
void GameSession::OnConnected()
{
	GSessionManager.Add(static_pointer_cast<GameSession>(shared_from_this()));
}

void GameSession::OnDisconnected()
{
	GSessionManager.Remove(static_pointer_cast<GameSession>(shared_from_this()));
}

int32 GameSession::OnRecv(BYTE* buffer, int32 len)
{
	// Echo
	cout << "OnRecv Len = " << len << endl;
	cout << "OnRecv Data = " << buffer << endl;

	SendBufferRef sendBuffer = SendBufferRef(new SendBuffer(4096));
	sendBuffer->CopyData(buffer, len);

	GSessionManager.Broadcast(sendBuffer);

	return len;
}

void GameSession::OnSend(int32 len)
{
	cout << "OnSend Len = " << len << endl;
}

/*----------------------
	Game Packet Session
	With PacketHeader
----------------------*/
void GamePacketSession::OnConnected()
{
	GPacketSessionManager.Add(static_pointer_cast<GamePacketSession>(shared_from_this()));
}

void GamePacketSession::OnDisconnected()
{
	GPacketSessionManager.Remove(static_pointer_cast<GamePacketSession>(shared_from_this()));
}

void GamePacketSession::OnRecvPacket(BYTE* buffer, int32 len)
{
	PacketSessionRef session = GetPacketSessionRef();
	PacketHeader* header = reinterpret_cast<PacketHeader*>(buffer);

	cout << "Packet Id: " << header->id << ", SIze: " << header->size << endl;
	cout << &buffer[4] << endl;

	SendBufferRef sendBuffer = SendBufferRef(new SendBuffer(4096));
	reinterpret_cast<PacketHeader*>(sendBuffer->Buffer())->id = reinterpret_cast<PacketHeader*>(buffer)->id;
	reinterpret_cast<PacketHeader*>(sendBuffer->Buffer())->size = len;
	sendBuffer->CopyPacket(&sendBuffer->Buffer()[4], buffer, header->size - sizeof(PacketHeader));

	GPacketSessionManager.Broadcast(sendBuffer);

	// TODO: PacketId 대역 체크
	//ClientPacketHandler::HandlePacket(session, buffer, len);
}

void GamePacketSession::OnSend(int32 len)
{

}

/*-------------------
	Game Protobuf Session
	With PakcetHeader
-------------------*/
void GameProtobufSession::OnConnected()
{
	GProtobufSessionManager.Add(static_pointer_cast<GameProtobufSession>(shared_from_this()));
}

void GameProtobufSession::OnDisconnected()
{
	GProtobufSessionManager.Remove(static_pointer_cast<GameProtobufSession>(shared_from_this()));
}

void GameProtobufSession::OnRecvPacket(BYTE* buffer, int32 len)
{
	PacketSessionRef session = GetPacketSessionRef();
	PacketHeader* recvHeader = reinterpret_cast<PacketHeader*>(buffer);

	cout << "Packet Id: " << recvHeader->id << ", SIze: " << recvHeader->size << endl;

	switch (recvHeader->id)
	{
	case PKT_C_TEST:
		Protocol::C_CHAT pkt;
		pkt.ParseFromArray(buffer + sizeof(PacketHeader), len - sizeof(PacketHeader));
		cout << pkt.msg() << endl;
		break;
	}

	Protocol::S_CHAT pkt;
	pkt.set_msg("Pong");
	uint16 packetId = PKT_S_TEST;
	SendProtobuf(pkt, packetId, this);

	//GProtobufSessionManager.Broadcast(sendBuffer);
}

void GameProtobufSession::OnSend(int32 len)
{
	cout << "OnSend Len = " << len << endl;
}

