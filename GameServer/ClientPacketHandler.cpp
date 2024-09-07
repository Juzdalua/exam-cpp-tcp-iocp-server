#include "pch.h"
#include "ClientPacketHandler.h"
#include "GameSession.h"
#include "Protocol.pb.h"
#include "AccountController.h"
#include "PlayerController.h"
#include "Player.h"
#include "Room.h"
#include "ItemController.h"

mutex _lock;
bool ClientPacketHandler::HandlePacket(BYTE* buffer, int32 len, GameProtobufSessionRef& session)
{
	PacketHeader* recvHeader = reinterpret_cast<PacketHeader*>(buffer);
	GRoom.CheckPlayers();
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

	uint16 packetId = PKT_S_TEST;
	Protocol::S_CHAT pkt;
	pkt.set_msg("Pong");
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

	shared_ptr<Account> account = AccountController::GetAccountByName(recvPkt.account().name());
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

	// Already login
	if (GRoom.IsLogin(pairAccountPlayer.second->GetPlayerId())) 
	{
		cout << pairAccountPlayer.second->GetPlayerId() << " is Already Login" << endl;
		auto errorPkt = new Protocol::ErrorObj();
		errorPkt->set_errorcode(-PKT_C_LOGIN);
		errorPkt->set_errormsg("Already Login");

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
	sendPlayer->set_name(pairAccountPlayer.second->GetPlayerName());
	sendPlayer->set_posx(pairAccountPlayer.second->GetPosX());
	sendPlayer->set_posy(pairAccountPlayer.second->GetPosY());
	sendPlayer->set_maxhp(pairAccountPlayer.second->GetMaxHP());
	sendPlayer->set_currenthp(pairAccountPlayer.second->GetCurrentHP());

	// Set Player in Session
	PlayerRef playerRef = make_shared<Player>(
		pairAccountPlayer.second->GetPlayerId(),
		pairAccountPlayer.second->GetAccountId(),
		pairAccountPlayer.second->GetPlayerName(),
		pairAccountPlayer.second->GetPosX(),
		pairAccountPlayer.second->GetPosY(),
		pairAccountPlayer.second->GetMaxHP(),
		pairAccountPlayer.second->GetCurrentHP()
	);
	playerRef->SetOwnerSession(session);

	// Set Session
	session->_player = playerRef;
	session->_accountId = recvAccount.id();

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

	// Get Player in Session
	PlayerRef player = session->_player;

	// Validation
	if (player->GetPlayerId() != recvPkt.playerid()) {
		uint16 packetId = PKT_S_ENTER_GAME;
		Protocol::S_ENTER_GAME pkt;
		pkt.set_success(false);
		pkt.set_toplayer(Protocol::TO_PLAYER_OWNER);
		session->Send(MakeSendBuffer(pkt, packetId));
		return false;
	}

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
	/*auto m = pkt.mutable_players();
	m->Add();*/
	GRoom.Broadcast(MakeSendBuffer(pkt, packetId));

	// Send all players in room to new player
	map<uint64, PlayerRef>* players = GRoom.GetPlayersInRoom();
	if (players->size() > 0) {
		for (const auto& pair : *players)
		{
			const PlayerRef _player = pair.second;
			Protocol::Player* repeatedPlayer = pkt.add_players();
			repeatedPlayer->set_id(_player->GetPlayerId());
			repeatedPlayer->set_accountid(_player->GetAccountId());
			repeatedPlayer->set_name(_player->GetPlayerName());
			repeatedPlayer->set_posx(_player->GetPosX());
			repeatedPlayer->set_posy(_player->GetPosY());
			repeatedPlayer->set_maxhp(_player->GetMaxHP());
			repeatedPlayer->set_currenthp(_player->GetCurrentHP());
		}
		pkt.set_toplayer(Protocol::TO_PLAYER_OWNER);

		cout << endl;
		session->Send(MakeSendBuffer(pkt, packetId));
	}
	GRoom.Enter(player); // WRITE_LOCK

	// Create Game Set
	lock_guard<mutex> lock(_lock);
	cout << "ROOM SIZE: " << GRoom.GetSize() << endl;
	Protocol::S_CREATE_ROOM createRoomPkt;
	uint16 createRoomPacketId = PKT_S_CREATE_ROOM;
	
	vector<shared_ptr<RoomItem>> roomItems = {};
	if (GRoom.GetSize() == 1)
	{
		// TODO -> MAKE Room ID
		//vector<shared_ptr<RoomItem>> roomItems = ItemController::GetRoomItemsByRoomId(1);
		roomItems = ItemController::GetRoomItemsByRoomId(1);
		GRoom.SetRoomItems(roomItems);
	}
	else
	{
		roomItems = GRoom.GetRoomItems();
	}

	for (shared_ptr<RoomItem>& roomItem : roomItems)
	{
		Protocol::RoomItem* repeatedItem = createRoomPkt.add_item();
		repeatedItem->set_roomid(roomItem->GetRoomId());
		repeatedItem->set_roomitemid(roomItem->GetRoomItemId());
		repeatedItem->set_posx(roomItem->GetPosX());
		repeatedItem->set_posy(roomItem->GetPosY());

		switch (roomItem->GetRoomItemState())
		{
		default:
		case RoomItemState::AVAILABLE:
			repeatedItem->set_state(Protocol::ROOM_ITEM_STATE_AVAILABLE);
			break;

		case RoomItemState::RESPAWN_PENDING:
			repeatedItem->set_state(Protocol::ROOM_ITEM_STATE_RESPAWN_PENDING);
			break;
		}

		auto sendItem = new Protocol::Item();
		sendItem->set_itemid(roomItem->GetItemId());
		sendItem->set_value(roomItem->GetItemValue());
		sendItem->set_amount(1);
		if (roomItem->GetItemEffect() == "HP")
		{
			sendItem->set_type(Protocol::ITEM_TYPE_HEAL);
			sendItem->set_effect(Protocol::ITEM_EFFECT_HP);
		}
		// TODO 
		else {

		}
		repeatedItem->set_allocated_item(sendItem);
	}

	session->Send(MakeSendBuffer(createRoomPkt, createRoomPacketId));

	return true;
}

/*--------------------
	Chat
--------------------*/
bool ClientPacketHandler::HandleChat(BYTE* buffer, int32 len, GameProtobufSessionRef& session)
{
	Protocol::C_CHAT recvPkt;
	recvPkt.ParseFromArray(buffer + sizeof(PacketHeader), len - sizeof(PacketHeader));

	// Validation
	if (session->_player->GetPlayerId() != recvPkt.playerid()) {
		// TODO Send Error
		cout << "[ERROR: " << session->_player->GetPlayerId() << "is Invalid ID" << "]" << endl;
		return false;
	}

	shared_ptr<Account> account = AccountController::GetAccountByPlayerId(recvPkt.playerid());

	uint16 packetId = PKT_S_CHAT;
	Protocol::S_CHAT pkt;
	pkt.set_type(recvPkt.type());
	pkt.set_playerid(recvPkt.playerid());
	pkt.set_playername(account->GetAccountName());
	pkt.set_msg(recvPkt.msg());

	// TODO Type check
	switch (recvPkt.type())
	{
		// nomal chat
	default:
	case Protocol::CHAT_TYPE_NORMAL:
	{
		GRoom.Broadcast(MakeSendBuffer(pkt, packetId));
	}
	break;

	case Protocol::CHAT_TYPE_PARTY:
	{
		// TODO check session -> Broadcast to party
		GRoom.Broadcast(MakeSendBuffer(pkt, packetId));
	}
	break;

	case Protocol::CHAT_TYPE_GUILD:
	{
		// TODO check session -> Broadcast to guild
		GRoom.Broadcast(MakeSendBuffer(pkt, packetId));
	}
	break;

	case Protocol::CHAT_TYPE_WHISPER:
	{
		// TODO check session -> Send
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

	// Validation
	if (session->_player->GetPlayerId() != recvPkt.playerid()) {
		// TODO Send Error
		cout << "[ERROR: " << session->_player->GetPlayerId() << "is Invalid ID" << "]" << endl;
		return false;
	}

	if (!GRoom.CanGo(session->_player->GetPlayerId(), recvPkt.posx(), recvPkt.posy())) {
		//TODO ERROR SEND
		cout << "CAN'T GO" << endl;
		return false;
	}
	
	// Update Session & Room
	session->_player->SetPosition(recvPkt.posx(), recvPkt.posy());
	GRoom.UpdateMove(recvPkt.playerid(), recvPkt.posx(), recvPkt.posy());

	// Update DB
	PlayerController::UpdateMove(recvPkt.playerid(), recvPkt.posx(), recvPkt.posy());

	// Broadcast players in room
	auto sendPlayer = new Protocol::Player();
	sendPlayer->set_id(session->_player->GetPlayerId());
	sendPlayer->set_accountid(session->_player->GetAccountId());
	sendPlayer->set_posx(session->_player->GetPosX());
	sendPlayer->set_posy(session->_player->GetPosY());
	sendPlayer->set_maxhp(session->_player->GetMaxHP());
	sendPlayer->set_currenthp(session->_player->GetCurrentHP());

	uint16 packetId = PKT_S_MOVE;
	Protocol::S_MOVE pkt;
	pkt.set_dir(recvPkt.dir());
	pkt.set_allocated_player(sendPlayer);
	GRoom.Broadcast(MakeSendBuffer(pkt, packetId));

	return true;
}

// SHOT
bool ClientPacketHandler::HandleShot(BYTE* buffer, int32 len, GameProtobufSessionRef& session)
{
	Protocol::C_SHOT recvPkt;
	recvPkt.ParseFromArray(buffer + sizeof(PacketHeader), len - sizeof(PacketHeader));

	cout << "[SHOT PLAYER: " << session->_player->GetPlayerId() << "]" << endl;
	// TODO DB LOG SAVE
	

	uint16 packetId = PKT_S_SHOT;
	Protocol::S_SHOT pkt;
	pkt.set_playerid(recvPkt.playerid());
	pkt.set_spawnposx(recvPkt.spawnposx());
	pkt.set_spawnposy(recvPkt.spawnposy());
	pkt.set_targetposx(recvPkt.targetposx());
	pkt.set_targetposy(recvPkt.targetposy());
	GRoom.Broadcast(MakeSendBuffer(pkt, packetId));

	return false;
}

// HIT
bool ClientPacketHandler::HandleHit(BYTE* buffer, int32 len, GameProtobufSessionRef& session)
{
	Protocol::C_HIT recvPkt;
	recvPkt.ParseFromArray(buffer + sizeof(PacketHeader), len - sizeof(PacketHeader));

	// Validation
	if (session->_player->GetPlayerId() != recvPkt.playerid()) {
		// TODO Send Error
		cout << "[ERROR: " << session->_player->GetPlayerId() << "is Invalid ID" << "]" << endl;
		return false;
	}
	
	uint64 currentHP = PlayerController::DecreaseHP(recvPkt.playerid(), recvPkt.damage());
	if (currentHP < 0) {
		// TODO error
		cout << "ERROR: currentHP = -1 " << endl;
		return false;
	}

	// Session & Room Update
	session->_player->DecreaseHP(recvPkt.damage());
	GRoom.UpdateCurrentHP(session->_player->GetPlayerId(), currentHP);
	cout << "[HIT PLAYER: " << session->_player->GetPlayerId() << ", CURRENTHP: " << currentHP << "]" << endl;

	uint16 packetId = PKT_S_HIT;
	Protocol::S_HIT pkt;
	if (currentHP == 0) {
		pkt.set_state(Protocol::PLAYER_STATE_DEAD);
	}
	else 
	{
		pkt.set_state(Protocol::PLAYER_STATE_LIVE);
	}
	pkt.set_currenthp(currentHP);
	pkt.set_playerid(session->_player->GetPlayerId());
	GRoom.Broadcast(MakeSendBuffer(pkt, packetId));

	return true;
}

// Eat Room Item
bool ClientPacketHandler::HandleEatRoomItem(BYTE* buffer, int32 len, GameProtobufSessionRef& session)
{
	Protocol::C_EAT_ROOM_ITEM recvPkt;
	recvPkt.ParseFromArray(buffer + sizeof(PacketHeader), len - sizeof(PacketHeader));

	// Validation
	if (session->_player->GetPlayerId() != recvPkt.playerid()) {
		// TODO Send Error
		cout << "[ERROR: " << session->_player->GetPlayerId() << "is Invalid ID" << "]" << endl;
		return false;
	}

	const Protocol::RoomItem& recvItem = recvPkt.item();
	shared_ptr<RoomItem> roomItem = ItemController::GetRoomItemByRoomItemId(recvItem.roomitemid());

	// Session & Room Update
	roomItem->EatItem();
	GRoom.UpdateRoomItem(roomItem);

	switch (recvItem.item().type())
	{
	default:
	case Protocol::ITEM_TYPE_HEAL:
		
		session->_player->HealHP(roomItem->GetItemValue());
		GRoom.UpdateCurrentHP(session->_player->GetPlayerId(), session->_player->GetCurrentHP());
		break;

	// TODO ADD MORE ITEM
	}

	// UPDATE DB WITH PLAYER
	PlayerController::UpdatePlayer(session->_player);

	auto sendPlayer = new Protocol::Player();
	sendPlayer->set_id(session->_player->GetPlayerId());
	sendPlayer->set_accountid(session->_player->GetAccountId());
	sendPlayer->set_posx(session->_player->GetPosX());
	sendPlayer->set_posy(session->_player->GetPosY());
	sendPlayer->set_maxhp(session->_player->GetMaxHP());
	sendPlayer->set_currenthp(session->_player->GetCurrentHP());

	uint16 packetId = PKT_S_EAT_ROOM_ITEM;
	Protocol::S_EAT_ROOM_ITEM pkt;
	pkt.set_allocated_player(sendPlayer);
	GRoom.Broadcast(MakeSendBuffer(pkt, packetId));

	// Respawn Room Item
	Protocol::S_CREATE_ROOM createRoomPkt;
	uint16 createRoomPacketId = PKT_S_CREATE_ROOM;

	Protocol::RoomItem* repeatedItem = createRoomPkt.add_item();
	repeatedItem->set_roomid(roomItem->GetRoomId());
	repeatedItem->set_roomitemid(roomItem->GetRoomItemId());
	repeatedItem->set_posx(roomItem->GetPosX());
	repeatedItem->set_posy(roomItem->GetPosY());

	auto sendItem = new Protocol::Item();
	sendItem->set_itemid(roomItem->GetItemId());
	sendItem->set_value(roomItem->GetItemValue());
	sendItem->set_amount(1);
	if (roomItem->GetItemEffect() == "HP")
	{
		sendItem->set_type(Protocol::ITEM_TYPE_HEAL);
		sendItem->set_effect(Protocol::ITEM_EFFECT_HP);
	}
	// TODO OTHER ITEM
	else {

	}
	repeatedItem->set_allocated_item(sendItem);
	
	thread([=]() {
		this_thread::sleep_for(3s);
		GRoom.Broadcast(MakeSendBuffer(createRoomPkt, createRoomPacketId));
		}).detach();

	return true;
}

// USE ITEM
bool ClientPacketHandler::HandleUseItem(BYTE* buffer, int32 len, GameProtobufSessionRef& session)
{
	//Protocol::C_USE_ITEM recvPkt;
	//recvPkt.ParseFromArray(buffer + sizeof(PacketHeader), len - sizeof(PacketHeader));

	//// Validation
	//if (session->_player->GetPlayerId() != recvPkt.playerid()) {
	//	// TODO Send Error
	//	cout << "[ERROR: " << session->_player->GetPlayerId() << "is Invalid ID" << "]" << endl;
	//	return false;
	//}

	//const Protocol::Item& recvItem = recvPkt.item();
	//shared_ptr<Item> item = ItemController::GetItemById(recvItem.itemid());

	

	return true;
}
