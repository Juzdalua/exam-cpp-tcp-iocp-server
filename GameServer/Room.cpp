#include "pch.h"
#include "Room.h"
#include "GameSession.h"
#include "ClientPacketHandler.h"
#include "EnumMap.h"
#include "ItemController.h"
#include "PlayerController.h"

Room GRoom;

Room::Room()
{
	_roomItems = ItemController::GetRoomItemsByRoomId(1);

	bool updateFlag = false;
	// Init Room
	for (auto& roomItem : _roomItems)
	{
		if (roomItem->GetState() == RoomItemState::RESPAWN_PENDING)
		{
			updateFlag = true;
			roomItem->SetState(RoomItemState::AVAILABLE);
			break;
		}
	}

	// Update DB
	if (updateFlag)
	{
		ItemController::InitRoomItems();
	}
}

Room::~Room()
{
	_players.clear();
	_roomItems.clear();
}

void Room::FlushJob()
{
	while (true)
	{
		JobRef job = _jobs.Pop();
		if (job == nullptr)
			break;

		job->Execute();
	}
}

void Room::Enter(PlayerRef player)
{
	_players[player->GetPlayerId()] = player;
}

void Room::Leave(PlayerRef player)
{
	_players.erase(player->GetPlayerId());
}

void Room::Broadcast(SendBufferRef sendBuffer)
{
	for (auto& p : _players)
	{
		p.second->GetOwnerSession()->Send(sendBuffer);
	}
}

void Room::SendToTargetPlayer(uint64 targetPlayerId, SendBufferRef sendBuffer)
{
	auto it = _players.find(targetPlayerId);
	if (it != _players.end() && it->second != nullptr)
	{
		it->second->GetOwnerSession()->Send(sendBuffer);
	}
}

void Room::CheckPlayers()
{
	cout << "[Check Room Count: " << _players.size() << "]" << endl;
}

bool Room::IsLogin(uint64 playerId)
{
	auto it = _players.find(playerId);
	if (it != _players.end() && it->second != nullptr)
	{
		return true;
	}
	return false;
}

uint64 Room::GetSize()
{
	return _players.size();
}

bool Room::CanGo(uint64 playerId, float posX, float posY)
{
	for (auto& pair : _players)
	{
		PlayerRef player = pair.second;

		if (playerId == pair.second->GetPlayerId())
			continue;

		if (player->GetPosX() == posX && player->GetPosY() == posY) {
			return false;
		}
	}
	return true;
}

void Room::UpdateMove(uint64 playerId, float posX, float posY)
{
	for (auto& pair : _players)
	{
		PlayerRef player = pair.second;
		if (player->GetPlayerId() == playerId)
		{
			player->SetPosition(posX, posY);
		}
	}
}

// HP
void Room::UpdateCurrentHP(uint64 playerId, uint64 currentHP)
{
	_players[playerId]->SetCurrentHP(currentHP);
}

// Room Item
void Room::SetRoomItems(const vector<shared_ptr<RoomItem>>& roomItems)
{
	_roomItems.clear();
	for (const auto& item : roomItems) {
		_roomItems.push_back(make_shared<RoomItem>(*item));
	}
}

void Room::UpdateRoomItem(const shared_ptr<RoomItem>& roomItem)
{
	for (auto& item : _roomItems)
	{
		if (item->GetRoomItemId() == roomItem->GetRoomItemId())
		{
			RoomItemState state = roomItem->GetState();

			item = make_shared<RoomItem>(
				roomItem->GetRoomId(),
				roomItem->GetRoomItemId(),
				roomItem->GetPosX(),
				roomItem->GetPosY(),
				roomItem->GetItemId(),
				roomItem->GetItemName(),
				roomItem->GetItemEffect(),
				roomItem->GetItemValue(),
				roomItem->GetState()
			);
			return;
		}
	}
}

void EnterRoomJob::Execute()
{
	_room.Enter(_player);

	uint16 packetId = PKT_S_ENTER_GAME;
	Protocol::S_ENTER_GAME pkt;
	pkt.set_success(true);

	map<uint64, PlayerRef>* players = GRoom.GetPlayersInRoom();

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

	GRoom.PushJob(make_shared<SendToRoomTargetPlayerJob>(GRoom, _player->GetPlayerId(), MakeSendBuffer(pkt, packetId)));
}

void LeaveRoomJob::Execute()
{
	_room.Leave(_player);
}

void BroadcastRoomJob::Execute()
{
	_room.Broadcast(_sendBuffer);
}

void SendToRoomTargetPlayerJob::Execute()
{
	_room.SendToTargetPlayer(_targetPlayerId, _sendBuffer);
}

void SendWithSessionJob::Execute()
{
	_session->Send(_sendBuffer);
}


void IsLoginRoomPlayerJob::Execute()
{
	if (_room.IsLogin(_player->GetPlayerId()))
	{
		auto errorPkt = new Protocol::ErrorObj();
		errorPkt->set_errorcode(ErrorCode::ERROR_S_LOGIN_SESSION);
		errorPkt->set_errormsg("Already Login");

		uint16 packetId = PKT_S_LOGIN;
		Protocol::S_LOGIN pkt;
		pkt.set_success(false);
		pkt.set_allocated_player(nullptr);
		pkt.set_allocated_error(errorPkt);
		GRoom.PushJob(make_shared<SendToRoomTargetPlayerJob>(GRoom, _player->GetPlayerId(), MakeSendBuffer(pkt, packetId)));
	}
	else
	{
		// Set Send Player Info
		auto sendPlayer = new Protocol::Player();
		sendPlayer->set_id(_player->GetPlayerId());
		sendPlayer->set_accountid(_player->GetAccountId());
		sendPlayer->set_name(_player->GetPlayerName());
		sendPlayer->set_posx(_player->GetPosX());
		sendPlayer->set_posy(_player->GetPosY());
		sendPlayer->set_maxhp(_player->GetMaxHP());
		sendPlayer->set_currenthp(_player->GetCurrentHP());

		// Set Player in Session
		PlayerRef playerRef = make_shared<Player>(
			_player->GetPlayerId(),
			_player->GetAccountId(),
			_player->GetPlayerName(),
			_player->GetPosX(),
			_player->GetPosY(),
			_player->GetMaxHP(),
			_player->GetCurrentHP()
		);
		playerRef->SetOwnerSession(_session);

		// Set Session
		_session->_player = playerRef;
		_session->_accountId = _player->GetAccountId();

		// Set SendBuffer
		uint16 packetId = PKT_S_LOGIN;
		Protocol::S_LOGIN pkt;
		pkt.set_success(true);
		pkt.set_allocated_player(sendPlayer);
		GRoom.PushJob(make_shared<SendWithSessionJob>(GRoom, _session, MakeSendBuffer(pkt, packetId)));
	}
}

void CreateRoomJob::Execute()
{
	Protocol::S_CREATE_ROOM pkt;
	uint16 packetId = PKT_S_CREATE_ROOM;

	vector<shared_ptr<RoomItem>> roomItems = GRoom.GetRoomItems();

	for (shared_ptr<RoomItem>& roomItem : roomItems)
	{
		Protocol::RoomItem* repeatedItem = pkt.add_item();
		repeatedItem->set_roomid(roomItem->GetRoomId());
		repeatedItem->set_roomitemid(roomItem->GetRoomItemId());
		repeatedItem->set_posx(roomItem->GetPosX());
		repeatedItem->set_posy(roomItem->GetPosY());
		repeatedItem->set_state(EnumMap::RoomItemStateProtocolMap(roomItem->GetRoomItemState()));

		auto sendItem = new Protocol::Item();
		sendItem->set_itemid(roomItem->GetItemId());
		sendItem->set_value(roomItem->GetItemValue());
		sendItem->set_amount(1);
		sendItem->set_type(EnumMap::ItemTypeProtocolMap(roomItem->GetItemEffect()));
		sendItem->set_effect(EnumMap::ItemEffectProtocolMap(roomItem->GetItemEffect()));

		repeatedItem->set_allocated_item(sendItem);
	}

	GRoom.PushJob(make_shared<SendToRoomTargetPlayerJob>(GRoom, _targetPlayerId, MakeSendBuffer(pkt, packetId)));
}

void MoveRoomJob::Execute()
{
	if (!GRoom.CanGo(_player->GetPlayerId(), _recvPkt.posx(), _recvPkt.posy())) {
		auto errorPkt = new Protocol::ErrorObj();
		errorPkt->set_errorcode(ErrorCode::ERROR_S_MOVE);
		errorPkt->set_errormsg("Can't go");

		uint16 packetId = PKT_S_MOVE;
		Protocol::S_MOVE pkt;
		pkt.set_success(false);
		pkt.set_allocated_error(errorPkt);

		GRoom.PushJob(make_shared<SendToRoomTargetPlayerJob>(GRoom, _player->GetPlayerId(), MakeSendBuffer(pkt, packetId)));
		return;
	}

	// Update Session & Room
	_player->SetPosition(_recvPkt.posx(), _recvPkt.posy());
	GRoom.UpdateMove(_recvPkt.playerid(), _recvPkt.posx(), _recvPkt.posy());

	// Update DB
	auto copyPkt = _recvPkt;
	thread([copyPkt]() {
		PlayerController::UpdateMove(copyPkt.playerid(), copyPkt.posx(), copyPkt.posy());
		}).detach();

	// Broadcast players in room
	auto sendPlayer = new Protocol::Player();
	sendPlayer->set_id(_player->GetPlayerId());
	sendPlayer->set_accountid(_player->GetAccountId());
	sendPlayer->set_posx(_player->GetPosX());
	sendPlayer->set_posy(_player->GetPosY());
	sendPlayer->set_maxhp(_player->GetMaxHP());
	sendPlayer->set_currenthp(_player->GetCurrentHP());

	uint16 packetId = PKT_S_MOVE;
	Protocol::S_MOVE pkt;
	pkt.set_success(true);
	pkt.set_dir(_recvPkt.dir());
	pkt.set_allocated_player(sendPlayer);
	GRoom.PushJob(make_shared<BroadcastRoomJob>(GRoom, MakeSendBuffer(pkt, packetId)));
}

void HitRoomJob::Execute()
{
	_player->DecreaseHP(_damage);
	GRoom.UpdateCurrentHP(_player->GetPlayerId(), _currentHP);

	uint16 packetId = PKT_S_HIT;
	Protocol::S_HIT pkt;
	if (_currentHP == 0) {
		pkt.set_state(Protocol::PLAYER_STATE_DEAD);
	}
	else
	{
		pkt.set_state(Protocol::PLAYER_STATE_LIVE);
	}
	pkt.set_currenthp(_currentHP);
	pkt.set_playerid(_player->GetPlayerId());

	GRoom.PushJob(make_shared<BroadcastRoomJob>(GRoom, MakeSendBuffer(pkt, packetId)));
}

void UpdateItemRoomJob::Execute()
{
	if (_status != RoomItemState::NONE)
	{
		_roomItem->SetState(_status);
	}
	// _roomItem is null ERROR
	GRoom.UpdateRoomItem(_roomItem);
	thread([=]() {
		ItemController::UpdateRoomItemByRoomItem(_roomItem);
		}).detach();
}

void EatItemRoomJob::Execute()
{
	_roomItem->EatItem();
	//GRoom.PushJob(make_shared<UpdateItemRoomJob>(GRoom, _roomItem, RoomItemState::NONE));

	GRoom.UpdateRoomItem(_roomItem);

	auto copyRoomItem = _roomItem;
	thread([=]() {
		ItemController::UpdateRoomItemByRoomItem(copyRoomItem);
		}).detach();

	switch (_itemType)
	{
	default:
	case Protocol::ITEM_TYPE_HEAL:

		_player->HealHP(_roomItem->GetItemValue());
		GRoom.UpdateCurrentHP(_player->GetPlayerId(), _player->GetCurrentHP());
		break;
	}

	auto copyPlayer = _player;
	thread([=]() {
		PlayerController::UpdatePlayer(copyPlayer);
		}).detach();

	auto sendPlayer = new Protocol::Player();
	sendPlayer->set_id(_player->GetPlayerId());
	sendPlayer->set_accountid(_player->GetAccountId());
	sendPlayer->set_posx(_player->GetPosX());
	sendPlayer->set_posy(_player->GetPosY());
	sendPlayer->set_maxhp(_player->GetMaxHP());
	sendPlayer->set_currenthp(_player->GetCurrentHP());

	uint16 packetId = PKT_S_EAT_ROOM_ITEM;
	Protocol::S_EAT_ROOM_ITEM pkt;
	pkt.set_allocated_player(sendPlayer);
	GRoom.PushJob(make_shared<BroadcastRoomJob>(GRoom, MakeSendBuffer(pkt, packetId)));

	thread([copyRoomItem]() {
		this_thread::sleep_for(3s);

		Protocol::S_CREATE_ROOM createRoomPkt;
		uint16 createRoomPacketId = PKT_S_CREATE_ROOM;

		Protocol::RoomItem* repeatedItem = createRoomPkt.add_item();
		repeatedItem->set_roomid(copyRoomItem->GetRoomId());
		repeatedItem->set_roomitemid(copyRoomItem->GetRoomItemId());
		repeatedItem->set_posx(copyRoomItem->GetPosX());
		repeatedItem->set_posy(copyRoomItem->GetPosY());
		repeatedItem->set_state(Protocol::ROOM_ITEM_STATE_AVAILABLE);

		//GRoom.PushJob(make_shared<UpdateItemRoomJob>(GRoom, _roomItem, RoomItemState::AVAILABLE));
		copyRoomItem->SetState(RoomItemState::AVAILABLE);
		GRoom.UpdateRoomItem(copyRoomItem);

		thread([=]() {
			ItemController::UpdateRoomItemByRoomItem(copyRoomItem);
			}).detach();

		auto sendItem = new Protocol::Item();
		sendItem->set_itemid(copyRoomItem->GetItemId());
		sendItem->set_value(copyRoomItem->GetItemValue());
		sendItem->set_amount(1);

		sendItem->set_type(EnumMap::ItemTypeProtocolMap(copyRoomItem->GetItemEffect()));
		sendItem->set_effect(EnumMap::ItemEffectProtocolMap(copyRoomItem->GetItemEffect()));
		repeatedItem->set_allocated_item(sendItem);

		GRoom.PushJob(make_shared<BroadcastRoomJob>(GRoom, MakeSendBuffer(createRoomPkt, createRoomPacketId)));
		}).detach();
}
