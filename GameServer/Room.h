#pragma once

class Room
{
public:
	void Enter(PlayerRef player);
	void Leave(PlayerRef player);
	void Broadcast(SendBufferRef sendBuffer);

	bool IsLogin(uint64 playerId);

	uint64 GetSize();
	map<uint64, PlayerRef>* GetPlayersInRoom() {return &_players;}

	bool CanGo(uint64 playerId, float posX, float posY);
	void UpdateMove(uint64 playerId, float posX, float posY);

	void UpdateCurrentHP(uint64 playerId, uint64 currentHP);

	void CheckPlayers();

private:
	//USE_LOCK;
	mutex _lock;
	map<uint64, PlayerRef> _players;
};

extern Room GRoom;