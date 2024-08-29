#include "pch.h"
#include "Room.h"
#include "Player.h"
#include "GameSession.h"

Room GRoom;

void Room::Enter(PlayerRef player)
{
	//WRITE_LOCK;
	lock_guard<mutex> lock(_lock);
	_players[player->playerId] = player;
}

void Room::Leave(PlayerRef player)
{
	//WRITE_LOCK;
	lock_guard<mutex> lock(_lock);
	_players.erase(player->playerId);
}

void Room::Broadcast(SendBufferRef sendBuffer)
{
	//WRITE_LOCK;
	lock_guard<mutex> lock(_lock);
	for (auto& p : _players)
	{
		p.second->ownerSession->Send(sendBuffer);
	}
}
