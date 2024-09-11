#include "pch.h"
#include "ClientPacketHandler.h"
#include "GameSession.h"
#include "Protocol.pb.h"
#include "AccountController.h"
#include "PlayerController.h"
#include "Player.h"
#include "Room.h"
#include "ItemController.h"
#include "EnumMap.h"

mutex _lock;

bool isInvalidId(GameProtobufSessionRef& session, uint64 pktPlayerId, ErrorCode errorCode)
{
	if (session->_player->GetPlayerId() != pktPlayerId) {
		DebugLog::PrintColorText(LogColor::RED, "[ERROR: ", to_string(session->_player->GetPlayerId()), false, false);
		DebugLog::PrintColorText(LogColor::RED, "is Invalid ID]", "", false, true);

		auto errorPkt = new Protocol::ErrorObj();
		errorPkt->set_errorcode(errorCode);
		errorPkt->set_errormsg("Invalid ID");

		uint16 packetId = PKT_S_INVALID_ID;
		Protocol::S_INVALID_ID pkt;
		pkt.set_playerid(session->_player->GetPlayerId());
		pkt.set_success(false);
		pkt.set_allocated_error(errorPkt);

		GRoom.PushJob(make_shared<SendToRoomTargetPlayerJob>(GRoom, session->_player->GetPlayerId(), MakeSendBuffer(pkt, packetId)));
		
		return true;
	}
	return false;
}

bool ClientPacketHandler::HandlePacket(BYTE* buffer, int32 len, GameProtobufSessionRef& session)
{
	GRoom.CheckPlayers();
	PacketHeader* recvHeader = reinterpret_cast<PacketHeader*>(buffer);

	switch (recvHeader->id)
	{
	case PKT_C_PING:
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

	case PKT_C_MOVE:
		ClientPacketHandler::HandleMove(buffer, len, session);
		break;

	case PKT_C_SHOT:
		ClientPacketHandler::HandleShot(buffer, len, session);
		break;

	case PKT_C_HIT:
		ClientPacketHandler::HandleHit(buffer, len, session);
		break;

	case PKT_C_EAT_ROOM_ITEM:
		ClientPacketHandler::HandleEatRoomItem(buffer, len, session);
		break;

	case PKT_C_USE_ITEM:
		ClientPacketHandler::HandleUseItem(buffer, len, session);
		break;

	case PKT_C_CREATE_PARTY:
		ClientPacketHandler::HandleCreateParty(buffer, len, session);
		break;

	case PKT_C_JOIN_PARTY:
		ClientPacketHandler::HandleJoinParty(buffer, len, session);
		break;

	case PKT_C_WITHDRAW_PARTY:
		ClientPacketHandler::HandleWithdrawParty(buffer, len, session);
		break;

	case PKT_C_MY_PARTY:
		ClientPacketHandler::HandleGetMyParty(buffer, len, session);
		break;

	case PKT_C_ALL_PARTY:
		ClientPacketHandler::HandleGetAllParty(buffer, len, session);
		break;
	}

	return false;
}

bool ClientPacketHandler::HandleDisconnect(GameProtobufSessionRef& session)
{
	uint16 packetId = PKT_S_DISCONNECT;
	Protocol::S_DISCONNECT pkt;
	pkt.set_playerid(session->_player->GetPlayerId());
	GRoom.PushJob(make_shared<BroadcastRoomJob>(GRoom, MakeSendBuffer(pkt, packetId)));

	return true;
}

/*--------------------
	Test Ping-Pong
--------------------*/
bool ClientPacketHandler::HandleTest(BYTE* buffer, int32 len, GameProtobufSessionRef& session)
{
	Protocol::C_CHAT recvPkt;
	recvPkt.ParseFromArray(buffer + sizeof(PacketHeader), len - sizeof(PacketHeader));

	uint16 packetId = PKT_S_PING;
	Protocol::S_CHAT pkt;
	pkt.set_msg("Pong");
	GRoom.PushJob(make_shared<SendWithSessionJob>(GRoom, session, MakeSendBuffer(pkt, packetId)));

	return true;
}

/*--------------------
	Signup
--------------------*/
bool ClientPacketHandler::HandleSignup(BYTE* buffer, int32 len, GameProtobufSessionRef& session)
{
	Protocol::C_SIGNUP recvPkt;
	recvPkt.ParseFromArray(buffer + sizeof(PacketHeader), len - sizeof(PacketHeader));

	shared_ptr<Account> account = AccountController::GetAccountByName(recvPkt.account().name());
	if (account == nullptr)
	{
		bool res = AccountController::CreateAccount(recvPkt.account().id(), recvPkt.account().name(), recvPkt.account().password());
		if (res)
		{
			uint16 packetId = PKT_S_SIGNUP;
			Protocol::S_SIGNUP pkt;
			pkt.set_success(true);
			GRoom.PushJob(make_shared<SendWithSessionJob>(GRoom, session, MakeSendBuffer(pkt, packetId)));
			return true;
		}
	}

	auto errorPkt = new Protocol::ErrorObj();
	errorPkt->set_errorcode(ErrorCode::ERROR_S_SIGNUP);
	errorPkt->set_errormsg("ID has exists");

	uint16 packetId = PKT_S_SIGNUP;
	Protocol::S_SIGNUP ePkt;
	ePkt.set_success(false);
	ePkt.set_allocated_error(errorPkt);
	GRoom.PushJob(make_shared<SendWithSessionJob>(GRoom, session, MakeSendBuffer(ePkt, packetId)));

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
	pair<shared_ptr<Account>, shared_ptr<Player>> pairAccountPlayer = AccountController::GetAccountAndPlayerByName(recvAccount.name());

	// Invalid Id
	if (pairAccountPlayer.first == nullptr) {
		auto errorPkt = new Protocol::ErrorObj();
		errorPkt->set_errorcode(ErrorCode::ERROR_S_LOGIN_ID);
		errorPkt->set_errormsg("ID not exists");

		uint16 packetId = PKT_S_LOGIN;
		Protocol::S_LOGIN pkt;
		pkt.set_success(false);
		pkt.set_allocated_player(nullptr);
		pkt.set_allocated_error(errorPkt); // 소유권을 protobuf에 넘긴다 -> 메모리 해제를 자동으로 함.
		GRoom.PushJob(make_shared<SendWithSessionJob>(GRoom, session, MakeSendBuffer(pkt, packetId)));

		return false;
	}

	// Invalid Password
	if (pairAccountPlayer.first->GetHashedPwd() != recvAccount.password())
	{
		auto errorPkt = new Protocol::ErrorObj();
		errorPkt->set_errorcode(ErrorCode::ERROR_S_LOGIN_PWD);
		errorPkt->set_errormsg("Password not correct");

		uint16 packetId = PKT_S_LOGIN;
		Protocol::S_LOGIN pkt;
		pkt.set_success(false);
		pkt.set_allocated_player(nullptr);
		pkt.set_allocated_error(errorPkt);
		GRoom.PushJob(make_shared<SendWithSessionJob>(GRoom, session, MakeSendBuffer(pkt, packetId)));

		return false;
	}

	// Already login
	GRoom.PushJob(make_shared<IsLoginRoomPlayerJob>(GRoom, pairAccountPlayer.second, session));

	return true;
}

/*--------------------
	Enter Game
--------------------*/
bool ClientPacketHandler::HandleEnterGame(BYTE* buffer, int32 len, GameProtobufSessionRef& session)
{
	Protocol::C_ENTER_GAME recvPkt;
	recvPkt.ParseFromArray(buffer + sizeof(PacketHeader), len - sizeof(PacketHeader));

	// Get Player in Session
	PlayerRef player = session->_player;

	if(isInvalidId(session, recvPkt.playerid(), ErrorCode::ERROR_S_ENTER_GAME))
		return false;

	// Broadcast new player info to all players in room
	uint16 packetId = PKT_S_ENTER_GAME;
	Protocol::S_ENTER_GAME pkt;
	pkt.set_success(true);
	pkt.set_toplayer(Protocol::TO_PLAYER_ALL);
	{
		Protocol::Player* repeatPlayer = pkt.add_players();
		repeatPlayer->set_id(player->GetPlayerId());
		repeatPlayer->set_accountid(player->GetAccountId());
		repeatPlayer->set_name(player->GetPlayerName());
		repeatPlayer->set_posx(player->GetPosX());
		repeatPlayer->set_posy(player->GetPosY());
		repeatPlayer->set_maxhp(player->GetMaxHP());
		repeatPlayer->set_currenthp(player->GetCurrentHP());
	}
	
	GRoom.PushJob(make_shared<BroadcastRoomJob>(GRoom, MakeSendBuffer(pkt, packetId)));

	// Room Enter & Broadcast Room players info
	GRoom.PushJob(make_shared<EnterRoomJob>(GRoom, player));

	// Create Game Set
	GRoom.PushJob(make_shared<CreateRoomJob>(GRoom, session->_player->GetPlayerId()));

	return true;
}

// Chat
bool ClientPacketHandler::HandleChat(BYTE* buffer, int32 len, GameProtobufSessionRef& session)
{
	Protocol::C_CHAT recvPkt;
	recvPkt.ParseFromArray(buffer + sizeof(PacketHeader), len - sizeof(PacketHeader));

	if (isInvalidId(session, recvPkt.playerid(), ErrorCode::ERROR_S_CHAT))
		return false;

	shared_ptr<Account> account = AccountController::GetAccountByPlayerId(recvPkt.playerid());

	uint16 packetId = PKT_S_CHAT;
	Protocol::S_CHAT pkt;
	pkt.set_type(recvPkt.type());
	pkt.set_playerid(recvPkt.playerid());
	pkt.set_playername(account->GetAccountName());
	pkt.set_msg(recvPkt.msg());
	pkt.set_success(true);

	switch (recvPkt.type())
	{
	default:
	case Protocol::CHAT_TYPE_NORMAL:
		GRoom.PushJob(make_shared<BroadcastRoomJob>(GRoom, MakeSendBuffer(pkt, packetId)));
	break;

	case Protocol::CHAT_TYPE_WHISPER:
	{
		uint64 recvPlayerId = recvPkt.targetid();
		pkt.set_targetid(recvPkt.targetid());

		GRoom.PushJob(make_shared<SendToRoomTargetPlayerJob>(GRoom, recvPlayerId, MakeSendBuffer(pkt, packetId)));
		GRoom.PushJob(make_shared<SendToRoomTargetPlayerJob>(GRoom, session->_player->GetPlayerId(), MakeSendBuffer(pkt, packetId)));
	}
	break;

	case Protocol::CHAT_TYPE_PARTY:
	{
		vector<shared_ptr<Player>> myPartyPlayer = PlayerController::GetPartyPlayersByPlayerId(recvPkt.playerid());
		pkt.set_targetid(recvPkt.targetid());
		if (myPartyPlayer.size() > 0)
		{
			for (auto& partyPlayer : myPartyPlayer)
			{
				GRoom.PushJob(make_shared<SendToRoomTargetPlayerJob>(GRoom, partyPlayer->GetPlayerId(), MakeSendBuffer(pkt, packetId)));
			}
		}
	}
	break;

	case Protocol::CHAT_TYPE_GUILD:
	{
		// TODO check session -> Broadcast to guild
		//GRoom.Broadcast(MakeSendBuffer(pkt, packetId));
	}
	break;

	case Protocol::CHAT_TYPE_SYSTEM:
	{
		// TODO
	}
	break;
	}

	return true;
}

/*--------------------
	Move
--------------------*/
bool ClientPacketHandler::HandleMove(BYTE* buffer, int32 len, GameProtobufSessionRef& session)
{
	Protocol::C_MOVE recvPkt;
	recvPkt.ParseFromArray(buffer + sizeof(PacketHeader), len - sizeof(PacketHeader));

	if (isInvalidId(session, recvPkt.playerid(), ErrorCode::ERROR_S_MOVE))
		return false;

	GRoom.PushJob(make_shared<MoveRoomJob>(GRoom, session->_player, recvPkt));

	return true;
}

// SHOT
bool ClientPacketHandler::HandleShot(BYTE* buffer, int32 len, GameProtobufSessionRef& session)
{
	Protocol::C_SHOT recvPkt;
	recvPkt.ParseFromArray(buffer + sizeof(PacketHeader), len - sizeof(PacketHeader));

	uint16 packetId = PKT_S_SHOT;
	Protocol::S_SHOT pkt;
	pkt.set_success(true);
	pkt.set_playerid(recvPkt.playerid());
	pkt.set_spawnposx(recvPkt.spawnposx());
	pkt.set_spawnposy(recvPkt.spawnposy());
	pkt.set_targetposx(recvPkt.targetposx());
	pkt.set_targetposy(recvPkt.targetposy());
	GRoom.PushJob(make_shared<BroadcastRoomJob>(GRoom, MakeSendBuffer(pkt, packetId)));

	return false;
}

// HIT
bool ClientPacketHandler::HandleHit(BYTE* buffer, int32 len, GameProtobufSessionRef& session)
{
	Protocol::C_HIT recvPkt;
	recvPkt.ParseFromArray(buffer + sizeof(PacketHeader), len - sizeof(PacketHeader));

	if (isInvalidId(session, recvPkt.playerid(), ErrorCode::ERROR_S_HIT))
		return false;

	vector<shared_ptr<Player>> myPartyPlayers = PlayerController::GetPartyPlayersByPlayerId(recvPkt.playerid());
	if (myPartyPlayers.size() > 0)
	{
		for (auto& partyPlayer : myPartyPlayers)
		{
			if (partyPlayer->GetPlayerId() == recvPkt.shotplayerid())
			{
				auto errorPkt = new Protocol::ErrorObj();
				errorPkt->set_errorcode(ErrorCode::ERROR_S_HIT);
				errorPkt->set_errormsg("Shot My Party Player");

				uint16 packetId = PKT_S_MOVE;
				Protocol::S_MOVE pkt;
				pkt.set_success(false);
				pkt.set_allocated_error(errorPkt);

				GRoom.PushJob(make_shared<SendToRoomTargetPlayerJob>(GRoom, session->_player->GetPlayerId(), MakeSendBuffer(pkt, packetId)));
				return false;
			}
		}
	}

	uint64 currentHP = PlayerController::DecreaseHP(recvPkt.playerid(), recvPkt.damage());
	if (currentHP < 0) {
		auto errorPkt = new Protocol::ErrorObj();
		errorPkt->set_errorcode(ErrorCode::ERROR_S_HIT);
		errorPkt->set_errormsg("DB Error");

		uint16 packetId = PKT_S_HIT;
		Protocol::S_HIT pkt;
		pkt.set_success(false);
		pkt.set_allocated_error(errorPkt);

		GRoom.PushJob(make_shared<SendToRoomTargetPlayerJob>(GRoom, session->_player->GetPlayerId(), MakeSendBuffer(pkt, packetId)));
		return false;
	}

	// Session & Room Update
	GRoom.PushJob(make_shared<HitRoomJob>(GRoom, session->_player, recvPkt.damage(), currentHP));

	return true;
}

// Eat Room Item
bool ClientPacketHandler::HandleEatRoomItem(BYTE* buffer, int32 len, GameProtobufSessionRef& session)
{
	Protocol::C_EAT_ROOM_ITEM recvPkt;
	recvPkt.ParseFromArray(buffer + sizeof(PacketHeader), len - sizeof(PacketHeader));

	if (isInvalidId(session, recvPkt.playerid(), ErrorCode::ERROR_S_EAT_ROOM_ITEM))
		return false;

	const Protocol::RoomItem& recvItem = recvPkt.item();
	shared_ptr<RoomItem> roomItem = ItemController::GetRoomItemByRoomItemId(recvItem.roomitemid());

	// Session & Room & DB Update
	GRoom.PushJob(make_shared<EatItemRoomJob>(GRoom, session->_player, roomItem, recvItem.item().type()));

	// Respawn Room Item
	/*thread([=]() {
		this_thread::sleep_for(3s);
		
		Protocol::S_CREATE_ROOM createRoomPkt;
		uint16 createRoomPacketId = PKT_S_CREATE_ROOM;

		Protocol::RoomItem* repeatedItem = createRoomPkt.add_item();
		repeatedItem->set_roomid(roomItem->GetRoomId());
		repeatedItem->set_roomitemid(roomItem->GetRoomItemId());
		repeatedItem->set_posx(roomItem->GetPosX());
		repeatedItem->set_posy(roomItem->GetPosY());
		repeatedItem->set_state(Protocol::ROOM_ITEM_STATE_AVAILABLE);

		GRoom.PushJob(make_shared<UpdateItemRoomJob>(GRoom, roomItem, RoomItemState::AVAILABLE));

		auto sendItem = new Protocol::Item();
		sendItem->set_itemid(roomItem->GetItemId());
		sendItem->set_value(roomItem->GetItemValue());
		sendItem->set_amount(1);

		sendItem->set_type(EnumMap::ItemTypeProtocolMap(roomItem->GetItemEffect()));
		sendItem->set_effect(EnumMap::ItemEffectProtocolMap(roomItem->GetItemEffect()));
		repeatedItem->set_allocated_item(sendItem);

		GRoom.PushJob(make_shared<BroadcastRoomJob>(GRoom, MakeSendBuffer(createRoomPkt, createRoomPacketId)));
		}).detach();*/

	return true;
}

// USE ITEM
bool ClientPacketHandler::HandleUseItem(BYTE* buffer, int32 len, GameProtobufSessionRef& session)
{
	//Protocol::C_USE_ITEM recvPkt;
	//recvPkt.ParseFromArray(buffer + sizeof(PacketHeader), len - sizeof(PacketHeader));

	//if (isInvalidId(session, recvPkt.playerid(), ErrorCode::ERROR_S_USE_ITEM))
		//return false;

	//const Protocol::Item& recvItem = recvPkt.item();
	//shared_ptr<Item> item = ItemController::GetItemById(recvItem.itemid());

	return true;
}

bool ClientPacketHandler::HandleCreateParty(BYTE* buffer, int32 len, GameProtobufSessionRef& session)
{
	Protocol::C_USE_ITEM recvPkt;
	recvPkt.ParseFromArray(buffer + sizeof(PacketHeader), len - sizeof(PacketHeader));
	
	if (isInvalidId(session, recvPkt.playerid(), ErrorCode::ERROR_S_CREATE_PARTY))
		return false;

	int64 createPartyId = PlayerController::CreateParty(recvPkt.playerid());
	if (createPartyId == -1) {
		auto errorPkt = new Protocol::ErrorObj();
		errorPkt->set_errorcode(ErrorCode::ERROR_S_CREATE_PARTY);
		errorPkt->set_errormsg("Create Party Error");

		uint16 packetId = PKT_S_MOVE;
		Protocol::S_CREATE_PARTY pkt;
		pkt.set_success(false);
		pkt.set_allocated_error(errorPkt);

		GRoom.PushJob(make_shared<SendToRoomTargetPlayerJob>(GRoom, session->_player->GetPlayerId(), MakeSendBuffer(pkt, packetId)));
		return false;
	}

	uint16 packetId = PKT_S_CREATE_PARTY;
	Protocol::S_CREATE_PARTY pkt;
	pkt.set_success(true);
	pkt.set_partyid(createPartyId);
	GRoom.PushJob(make_shared<SendToRoomTargetPlayerJob>(GRoom, session->_player->GetPlayerId(), MakeSendBuffer(pkt, packetId)));

	return true;
}

bool ClientPacketHandler::HandleJoinParty(BYTE* buffer, int32 len, GameProtobufSessionRef& session)
{
	Protocol::C_JOIN_PARTY recvPkt;
	recvPkt.ParseFromArray(buffer + sizeof(PacketHeader), len - sizeof(PacketHeader));

	if (isInvalidId(session, recvPkt.playerid(), ErrorCode::ERROR_S_JOIN_PARTY))
		return false;

	bool join = PlayerController::JoinParty(recvPkt.playerid(), recvPkt.partyid());
	if (!join)
	{
		auto errorPkt = new Protocol::ErrorObj();
		errorPkt->set_errorcode(ErrorCode::ERROR_S_JOIN_PARTY);
		errorPkt->set_errormsg("JOIN PARTY ERROR");

		uint16 packetId = PKT_S_JOIN_PARTY;
		Protocol::S_JOIN_PARTY pkt;
		pkt.set_success(false);
		pkt.set_allocated_error(errorPkt);

		GRoom.PushJob(make_shared<SendToRoomTargetPlayerJob>(GRoom, session->_player->GetPlayerId(), MakeSendBuffer(pkt, packetId)));
		return false;
	}

	uint16 packetId = PKT_S_JOIN_PARTY;
	Protocol::S_JOIN_PARTY pkt;
	pkt.set_success(true);
	pkt.set_partyid(recvPkt.partyid());

	auto playerPkt = new Protocol::Player();
	playerPkt->set_id(recvPkt.playerid());
	playerPkt->set_name(session->_player->GetPlayerName());
	pkt.set_allocated_players(playerPkt);

	GRoom.PushJob(make_shared<BroadcastRoomJob>(GRoom, MakeSendBuffer(pkt, packetId)));

	return true;
}

bool ClientPacketHandler::HandleWithdrawParty(BYTE* buffer, int32 len, GameProtobufSessionRef& session)
{
	Protocol::C_WITHDRAW_PARTY recvPkt;
	recvPkt.ParseFromArray(buffer + sizeof(PacketHeader), len - sizeof(PacketHeader));

	if (isInvalidId(session, recvPkt.playerid(), ErrorCode::ERROR_S_WITHDRAW_PARTY))
		return false;

	uint64 withDrawPartyAndGetCount = PlayerController::WithdrawParty(recvPkt.playerid(), recvPkt.partyid());
	if (withDrawPartyAndGetCount == -1) {
		auto errorPkt = new Protocol::ErrorObj();
		errorPkt->set_errorcode(ErrorCode::ERROR_S_WITHDRAW_PARTY);
		errorPkt->set_errormsg("Withdraw Party Error");

		uint16 packetId = PKT_S_WITHDRAW_PARTY;
		Protocol::S_WITHDRAW_PARTY pkt;
		pkt.set_success(false);
		pkt.set_allocated_error(errorPkt);

		GRoom.PushJob(make_shared<SendToRoomTargetPlayerJob>(GRoom, session->_player->GetPlayerId(), MakeSendBuffer(pkt, packetId)));
		return false;
	}
	
	// Update Room
	if (withDrawPartyAndGetCount == 0) {
		PlayerController::CloseParty(recvPkt.partyid());
	}

	vector<shared_ptr<Player>> players = PlayerController::GetPartyPlayersByPartyId(recvPkt.partyid());

	uint16 packetId = PKT_S_WITHDRAW_PARTY;
	Protocol::S_WITHDRAW_PARTY pkt;
	pkt.set_success(true);
	pkt.set_partyid(recvPkt.partyid());
	pkt.set_withdrawplayerid(recvPkt.playerid());

	for (shared_ptr<Player>& player : players)
	{
		Protocol::Player* repeatedPlayer = pkt.add_players();
		repeatedPlayer->set_id(player->GetPlayerId());
		repeatedPlayer->set_name(player->GetPlayerName());
	}

	GRoom.PushJob(make_shared<BroadcastRoomJob>(GRoom, MakeSendBuffer(pkt, packetId)));

	return true;
}

bool ClientPacketHandler::HandleGetMyParty(BYTE* buffer, int32 len, GameProtobufSessionRef& session)
{
	Protocol::C_MY_PARTY recvPkt;
	recvPkt.ParseFromArray(buffer + sizeof(PacketHeader), len - sizeof(PacketHeader));

	if (isInvalidId(session, recvPkt.playerid(), ErrorCode::ERROR_S_MY_PARTY))
		return false;

	int64 partyId = PlayerController::GetMyPartyIdByPlayerId(recvPkt.playerid());
	vector<shared_ptr<Player>> players = PlayerController::GetPartyPlayersByPlayerId(recvPkt.playerid());

	uint16 packetId = PKT_S_MY_PARTY;
	Protocol::S_MY_PARTY pkt;
	pkt.set_success(true);
	pkt.set_partyid(partyId);
	
	if (partyId != 0) {
		for (shared_ptr<Player>& player : players)
		{
			Protocol::Player* repeatedPlayer = pkt.add_players();
			repeatedPlayer->set_id(player->GetPlayerId());
			repeatedPlayer->set_name(player->GetPlayerName());
		}
	}

	GRoom.PushJob(make_shared<SendToRoomTargetPlayerJob>(GRoom, session->_player->GetPlayerId(), MakeSendBuffer(pkt, packetId)));

	return true;
}

bool ClientPacketHandler::HandleGetAllParty(BYTE* buffer, int32 len, GameProtobufSessionRef& session)
{
	Protocol::C_MY_PARTY recvPkt;
	recvPkt.ParseFromArray(buffer + sizeof(PacketHeader), len - sizeof(PacketHeader));
	
	if (isInvalidId(session, recvPkt.playerid(), ErrorCode::ERROR_S_ALL_PARTY))
		return false;

	vector<pair<shared_ptr<Party>, shared_ptr<PartyPlayer>>> parties = PlayerController::GetAllParties();

	uint16 packetId = PKT_S_ALL_PARTY;
	Protocol::S_ALL_PARTY pkt;

	// Set Party
	map<uint64, Protocol::Party*> sendPartiesMap;
	for (auto& pair : parties) 
	{
		if (sendPartiesMap[pair.first->GetPartyId()] == nullptr) {
			Protocol::Party* repeatedParty = pkt.add_parties();
			repeatedParty->set_partyid(pair.first->GetPartyId());
			repeatedParty->set_partystatus(Protocol::PARTY_STATUS_AVAILABLE);
			sendPartiesMap[pair.first->GetPartyId()] = repeatedParty;
		}
	}

	// Set Party Player
	for (auto& pair : parties)
	{
		auto sendPartyPlayer = new Protocol::PartyPlayer();
		sendPartyPlayer->set_playerid(pair.second->GetPlayerId());

		Protocol::PartyPlayer* repeatedPartyPlayer = sendPartiesMap[pair.second->GetPartyId()]->add_partyplayers();
		repeatedPartyPlayer->set_playerid(pair.second->GetPlayerId());
	}
	
	pkt.set_success(true);
	GRoom.PushJob(make_shared<SendToRoomTargetPlayerJob>(GRoom, session->_player->GetPlayerId(), MakeSendBuffer(pkt, packetId)));

	return true;
}
