#include "pch.h"
#include "Room.h"
#include "Player.h"
#include "GameSession.h"

Room GRoom;

Room::Room()
{
}

Room::~Room()
{
	_players.clear();
}

void Room::Enter(PlayerRef player)
{
	lock_guard<mutex> lock(_lock);
	_players[player->GetPlayerId()] = player;
}

void Room::Leave(PlayerRef player)
{
	lock_guard<mutex> lock(_lock);
	_players.erase(player->GetPlayerId());
}

void Room::Broadcast(SendBufferRef sendBuffer)
{
	lock_guard<mutex> lock(_lock);
	for (auto& p : _players)
	{
		p.second->GetOwnerSession()->Send(sendBuffer);
	}
}

void Room::SendToTargetPlayer(uint64 targetPlayerId, SendBufferRef sendBuffer)
{
	lock_guard<mutex> lock(_lock);
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
	lock_guard<mutex> lock(_lock);
	for (auto& pair : _players)
	{
		PlayerRef player = pair.second;
		if (player->GetPlayerId() == playerId)
		{
			return true;
		}
	}
	return false;
}

uint64 Room::GetSize()
{
	lock_guard<mutex> lock(_lock);
	return _players.size();
}

bool Room::CanGo(uint64 playerId, float posX, float posY)
{
	lock_guard<mutex> lock(_lock);
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
	lock_guard<mutex> lock(_lock);
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
	lock_guard<mutex> lock(_lock);
	_players[playerId]->SetCurrentHP(currentHP);
}

// Room Item
void Room::SetRoomItems(const vector<shared_ptr<RoomItem>>& roomItems)
{
	lock_guard<mutex> lock(_lock);
	_roomItems.clear();
	for (const auto& item : roomItems) {
		_roomItems.push_back(make_shared<RoomItem>(*item));
	}
}

void Room::UpdateRoomItem(const shared_ptr<RoomItem>& roomItem)
{
	lock_guard<mutex> lock(_lock);
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
