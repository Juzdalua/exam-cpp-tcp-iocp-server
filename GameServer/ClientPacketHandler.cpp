#include "pch.h"
#include "ClientPacketHandler.h"
#include "Protocol.pb.h"
#include "GameSession.h"
#include "AccountController.h"

bool ClientPacketHandler::HandlePacket(BYTE* buffer, int32 len, GameProtobufSessionRef& session)
{
	PacketHeader* recvHeader = reinterpret_cast<PacketHeader*>(buffer);

	switch (recvHeader->id)
	{
	case PKT_C_TEST:
		ClientPacketHandler::HandleTest(buffer, len, session);
		break;

	case PKT_C_SIGNUP:
		ClientPacketHandler::HandleSignup(buffer, len, session);
		break;

	case PKT_C_LOGIN:
		ClientPacketHandler::HandleLogin(buffer, len, session);
		break;
	}

	return false;
}

bool ClientPacketHandler::HandleTest(BYTE* buffer, int32 len, GameProtobufSessionRef& session)
{
	Protocol::C_CHAT recvPkt;
	recvPkt.ParseFromArray(buffer + sizeof(PacketHeader), len - sizeof(PacketHeader));
	cout << recvPkt.msg() << endl;

	Protocol::S_CHAT pkt;
	pkt.set_msg("Pong");
	uint16 packetId = PKT_S_TEST;
	SendProtobuf(pkt, packetId, session);

	return true;
}

bool ClientPacketHandler::HandleSignup(BYTE* buffer, int32 len, GameProtobufSessionRef& session)
{
	
	return true;
}

bool ClientPacketHandler::HandleLogin(BYTE* buffer, int32 len, GameProtobufSessionRef& session)
{
	Protocol::C_LOGIN recvPkt;
	recvPkt.ParseFromArray(buffer + sizeof(PacketHeader), len - sizeof(PacketHeader));

	const Protocol::Account& recvAccount = recvPkt.account();
	std::cout << "Account ID: " << recvAccount.id() << ", Name: " << recvAccount.name() << ", Password: " << recvAccount.password() << std::endl;
	
	unique_ptr<Account> account = AccountController::GetAccountByName(recvAccount.name());
	if (account == nullptr) {
		cout << "ID not exists";
		return false;
	}

	cout << "ID: " << account->GetAccountId() << ", PWD: " << account->GetHashedPwd();

	return true;
}
