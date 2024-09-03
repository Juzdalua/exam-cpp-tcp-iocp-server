#include "pch.h"
#include "ClientPacketHandler.h"
#include "GameSession.h"
#include "Protocol.pb.h"
#include "AccountController.h"
#include "Player.h"
#include "Room.h"

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

	case PKT_C_ENTER_GAME:
		ClientPacketHandler::HandleEnterGame(buffer, len, session);
		break;

	case PKT_C_CHAT:
		ClientPacketHandler::HandleChat(buffer, len, session);
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
	uint16 packetId = PKT_S_TEST;
	session->Send(MakeSendBuffer(pkt, packetId));

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
			uint16 packetId = PKT_S_SIGNUP;
			Protocol::S_SIGNUP pkt;
			pkt.set_success(true);
			session->Send(MakeSendBuffer(pkt, packetId));
			return true;
		}
	}

	auto errorPkt = new Protocol::ErrorObj();
	errorPkt->set_errorcode(-PKT_C_SIGNUP);
	errorPkt->set_errormsg("ID has exists");

	uint16 packetId = PKT_S_SIGNUP;
	Protocol::S_SIGNUP pkt;
	pkt.set_success(false);
	pkt.set_allocated_error(errorPkt);
	session->Send(MakeSendBuffer(pkt, packetId));

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
		errorPkt->set_errorcode(-PKT_C_LOGIN);
		errorPkt->set_errormsg("ID not exists");

		uint16 packetId = PKT_S_LOGIN;
		Protocol::S_LOGIN pkt;
		pkt.set_success(false);
		pkt.set_allocated_player(nullptr);
		pkt.set_allocated_error(errorPkt); // 소유권을 protobuf에 넘긴다 -> 메모리 해제를 자동으로 함.
		session->Send(MakeSendBuffer(pkt, packetId));

		return false;
	}

	// Invalid Password
	if (pairAccountPlayer.first->GetHashedPwd() != recvAccount.password())
	{
		auto errorPkt = new Protocol::ErrorObj();
		errorPkt->set_errorcode(-PKT_C_LOGIN);
		errorPkt->set_errormsg("Password not correct");

		uint16 packetId = PKT_S_LOGIN;
		Protocol::S_LOGIN pkt;
		pkt.set_success(false);
		pkt.set_allocated_player(nullptr);
		pkt.set_allocated_error(errorPkt); 
		session->Send(MakeSendBuffer(pkt, packetId));

		return false;
	}

	// Set Send Player Info
	auto sendPlayer = new Protocol::Player();
	sendPlayer->set_id(pairAccountPlayer.second->GetPlayerId());
	sendPlayer->set_accountid(pairAccountPlayer.second->GetAccountId());
	sendPlayer->set_posx(pairAccountPlayer.second->GetPosX());
	sendPlayer->set_posy(pairAccountPlayer.second->GetPosY());
	sendPlayer->set_maxhp(pairAccountPlayer.second->GetMaxHP());
	sendPlayer->set_currenthp(pairAccountPlayer.second->GetCurrentHP());

	// Set Player
	PlayerRef playerRef = make_shared<Player>(
		pairAccountPlayer.second->GetPlayerId(), 
		pairAccountPlayer.second->GetAccountId(),
		pairAccountPlayer.second->GetPosX(),
		pairAccountPlayer.second->GetPosY(),
		pairAccountPlayer.second->GetMaxHP(),
		pairAccountPlayer.second->GetCurrentHP()
	);
	playerRef->SetOwnerSession(session);
	session->_player = playerRef;
	
	// Set SendBuffer
	uint16 packetId = PKT_S_LOGIN;
	Protocol::S_LOGIN pkt;
	pkt.set_success(true);
	pkt.set_allocated_player(sendPlayer);
	session->Send(MakeSendBuffer(pkt, packetId));

	return true;
}

/*--------------------
	Enter Game
--------------------*/
bool ClientPacketHandler::HandleEnterGame(BYTE* buffer, int32 len, GameProtobufSessionRef& session)
{
	Protocol::C_ENTER_GAME recvPkt;
	recvPkt.ParseFromArray(buffer + sizeof(PacketHeader), len - sizeof(PacketHeader));

	PlayerRef player = session->_player;

	GRoom.Enter(player); // WRITE_LOCK

	uint16 packetId = PKT_S_ENTER_GAME;
	Protocol::S_ENTER_GAME pkt;
	pkt.set_success(true);
	session->Send(MakeSendBuffer(pkt, packetId));

	return true;
}

/*--------------------
	Chat
--------------------*/
bool ClientPacketHandler::HandleChat(BYTE* buffer, int32 len, GameProtobufSessionRef& session)
{
	Protocol::C_CHAT recvPkt;
	recvPkt.ParseFromArray(buffer + sizeof(PacketHeader), len - sizeof(PacketHeader));

	uint16 packetId = PKT_S_CHAT;
	Protocol::S_CHAT pkt;
	pkt.set_msg(recvPkt.msg());
	GRoom.Broadcast(MakeSendBuffer(pkt, packetId)); // WRITE_LOCK

	return true;
}
