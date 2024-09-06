#include "pch.h"
#include "Room.h"
#include "Player.h"
#include "GameSession.h"

Room GRoom;

void Room::Enter(PlayerRef player)
{
	//WRITE_LOCK;
	lock_guard<mutex> lock(_lock);
	_players[player->GetPlayerId()] = player;
}

void Room::Leave(PlayerRef player)
{
	//WRITE_LOCK;
	lock_guard<mutex> lock(_lock);
	_players.erase(player->GetPlayerId());
}

void Room::Broadcast(SendBufferRef sendBuffer)
{
	//WRITE_LOCK;
	lock_guard<mutex> lock(_lock);
	for (auto& p : _players)
	{
		p.second->GetOwnerSession()->Send(sendBuffer);
	}
}

void Room::CheckPlayers()
{
	cout << "[Check Room Start / COUNT: " << _players.size() << "]" << endl;
	cout << "[Check Room END]" << endl;
}

bool Room::IsLogin(uint64 playerId)
{
	for (auto& pair : _players)
	{
		PlayerRef player = pair.second;
		if (player->GetPlayerId() == playerId)
		{
			cout << "ROOM: " << player->GetPlayerId() << " / LOGIN: " << playerId << endl;
			return true;
		}
	}
	return false;
}

bool Room::CanGo(uint64 playerId, float posX, float posY)
{
	lock_guard<mutex> lock(_lock);
	for (auto& pair : _players)
	{
		PlayerRef player = pair.second;
		if (playerId == pair.second->GetAccountId())
			continue;

		if (player->GetPosX() == posX && player->GetPosY() == posY) {
			cout << "[" <<"DIR: (" << posX << ", " << posY << "), ROOM: (" << player->GetPosX() << ", " << player->GetPosY() << ")]" << endl;
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