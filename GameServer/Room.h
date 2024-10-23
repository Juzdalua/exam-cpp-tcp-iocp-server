#pragma once
#include "Item.h"
#include "Player.h"
#include "Job.h"

class Room
{
public:
	Room();
	~Room();

public:
	void Enter(PlayerRef player);
	void Leave(PlayerRef player);
	void Broadcast(SendBufferRef sendBuffer);
	void SendToTargetPlayer(uint64 targetPlayerId, SendBufferRef sendBuffer);

	bool IsLogin(uint64 playerId);

	map<uint64, PlayerRef>* GetPlayersInRoom() { return &_players; }
	PlayerRef GetPlayerInRoomByPlayerId(uint64 playerId) { return _players[playerId]; }

	// Move
	bool CanGo(uint64 playerId, float posX, float posY);
	void UpdateMove(uint64 playerId, float posX, float posY);

	// HP
	void UpdateCurrentHP(uint64 playerId, uint64 currentHP);

	// Room Item
	vector <shared_ptr<RoomItem>> GetRoomItems() { return _roomItems; }
	void SetRoomItems(const vector<shared_ptr<RoomItem>>& roomItems);
	void UpdateRoomItem(const shared_ptr<RoomItem>& roomItem);

	// Party
	void CreateParty(uint64 partyId, PlayerRef playerRef);
	void WithdrawParty(uint64 partyId, PlayerRef playerRef);

private:
	//USE_LOCK;
	mutex _lock;
	map<uint64, PlayerRef> _players; // PlayerId
	vector <shared_ptr<RoomItem>> _roomItems;
	map<uint64, vector<PlayerRef>> _parties;
};

extern Room GRoom;
