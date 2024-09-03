#include "pch.h"
#include "ClientPacketHandler.h"
#include "GameSession.h"
#include "Protocol.pb.h"
#include "AccountController.h"
#include "Player.h"

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

/*--------------------
	Test Ping-Pong
--------------------*/
bool ClientPacketHandler::HandleTest(BYTE* buffer, int32 len, GameProtobufSessionRef& session)
{
	Protocol::C_CHAT recvPkt;
	recvPkt.ParseFromArray(buffer + sizeof(PacketHeader), len - sizeof(PacketHeader));
	cout << recvPkt.msg() << endl;

	Protocol::S_CHAT pkt;
	pkt.set_msg("Pong");
	uint16 packetId = PKT_ID::PKT_S_TEST;
	SendProtobuf(pkt, packetId, session);

	return true;
}

/*--------------------
	Signup
--------------------*/
bool ClientPacketHandler::HandleSignup(BYTE* buffer, int32 len, GameProtobufSessionRef& session)
{
	Protocol::C_SIGNUP recvPkt;
	recvPkt.ParseFromArray(buffer + sizeof(PacketHeader), len - sizeof(PacketHeader));

	unique_ptr<Account> account = AccountController::GetAccountByName(recvPkt.account().name());
	if (account == nullptr)
	{
		bool res = AccountController::CreateAccount(recvPkt.account().id(), recvPkt.account().name(), recvPkt.account().password());
		if (res)
		{
			Protocol::S_SIGNUP pkt;
			pkt.set_success(true);
			uint16 packetId = PKT_ID::PKT_S_SIGNUP;
			SendProtobuf(pkt, packetId, session);
			return true;
		}
	}

	auto errorPkt = new Protocol::ErrorObj();
	errorPkt->set_errorcode(-PKT_ID::PKT_C_SIGNUP);
	errorPkt->set_errormsg("ID has exists");

	Protocol::S_SIGNUP pkt;
	pkt.set_success(false);
	pkt.set_allocated_error(errorPkt);
	uint16 packetId = PKT_ID::PKT_S_SIGNUP;
	SendProtobuf(pkt, packetId, session);

	return false;
}

/*--------------------
	Login
--------------------*/
bool ClientPacketHandler::HandleLogin(BYTE* buffer, int32 len, GameProtobufSessionRef& session)
{
	Protocol::C_LOGIN recvPkt;
	recvPkt.ParseFromArray(buffer + sizeof(PacketHeader), len - sizeof(PacketHeader));

	const Protocol::Account& recvAccount = recvPkt.account();
	std::cout << "Account ID: " << recvAccount.id() << ", Name: " << recvAccount.name() << ", Password: " << recvAccount.password() << std::endl;

	pair<shared_ptr<Account>, shared_ptr<Player>> pairAccountPlayer = AccountController::GetAccountAndPlayerByName(recvAccount.name());
	
	// Invalid Id
	if (pairAccountPlayer.first == nullptr) {
		auto errorPkt = new Protocol::ErrorObj();
		errorPkt->set_errorcode(-PKT_ID::PKT_C_LOGIN);
		errorPkt->set_errormsg("ID not exists");

		Protocol::S_LOGIN pkt;
		pkt.set_success(false);
		pkt.set_allocated_player(nullptr);
		pkt.set_allocated_error(errorPkt); // 소유권을 protobuf에 넘긴다 -> 메모리 해제를 자동으로 함.
		uint16 packetId = PKT_ID::PKT_S_LOGIN;
		SendProtobuf(pkt, packetId, session);

		return false;
	}

	// Invalid Password
	if (pairAccountPlayer.first->GetHashedPwd() != recvAccount.password())
	{
		auto errorPkt = new Protocol::ErrorObj();
		errorPkt->set_errorcode(-PKT_ID::PKT_C_LOGIN);
		errorPkt->set_errormsg("Password not correct");

		Protocol::S_LOGIN pkt;
		pkt.set_success(false);
		pkt.set_allocated_player(nullptr);
		pkt.set_allocated_error(errorPkt); 
		uint16 packetId = PKT_ID::PKT_S_LOGIN;
		SendProtobuf(pkt, packetId, session);

		return false;
	}

	auto player = new Protocol::Player();
	player->set_id(pairAccountPlayer.second->GetPlayerId());
	player->set_accountid(pairAccountPlayer.second->GetAccountId());
	player->set_posx(pairAccountPlayer.second->GetPosX());
	player->set_posy(pairAccountPlayer.second->GetPosY());
	player->set_maxhp(pairAccountPlayer.second->GetMaxHP());
	player->set_currenthp(pairAccountPlayer.second->GetCurrentHP());

	Protocol::S_LOGIN pkt;
	pkt.set_success(true);
	pkt.set_allocated_player(player);
	uint16 packetId = PKT_ID::PKT_S_LOGIN;
	SendProtobuf(pkt, packetId, session);

	return true;
}
