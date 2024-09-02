#include "pch.h"
#include "ClientPacketHandler.h"
#include "Protocol.pb.h"
#include "GameSession.h"

bool ClientPacketHandler::HandlePacket(BYTE* buffer, int32 len, GameProtobufSessionRef& session)
{
	PacketHeader* recvHeader = reinterpret_cast<PacketHeader*>(buffer);

	switch (recvHeader->id)
	{
	case PKT_C_TEST:
	{
		Protocol::C_CHAT recvPkt;
		recvPkt.ParseFromArray(buffer + sizeof(PacketHeader), len - sizeof(PacketHeader));
		cout << recvPkt.msg() << endl;

		Protocol::S_CHAT pkt;
		pkt.set_msg("Pong");
		uint16 packetId = PKT_S_TEST;
		SendProtobuf(pkt, packetId, session);

		break;
	}

	case PKT_C_LOGIN:
	{

		break;
	}
	}

	return false;
}
