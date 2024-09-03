#pragma once

class Room
{
public:
	void Enter(PlayerRef player);
	void Leave(PlayerRef player);
	void Broadcast(SendBufferRef sendBuffer);

	map<uint64, PlayerRef>* GetPlayersInRoom() {return &_players;}

private:
	//USE_LOCK;
	mutex _lock;
	map<uint64, PlayerRef> _players;
};

extern Room GRoom;