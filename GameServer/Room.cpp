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
	cout << "[Check Room Start / COUNT: "<< _players.size() << "]" << endl;
	for (auto& pair : _players)
	{
		PlayerRef player = pair.second;
		cout << "ID: " << player->GetPlayerId() << " / Position: (" << player->GetPosX() << ", "<< player->GetPosY() << ")" << endl;
	}
	cout << "[Check Room END]" << endl;
}
