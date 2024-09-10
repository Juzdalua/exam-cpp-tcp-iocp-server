#pragma once
#include "Item.h"
#include "Player.h"

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

	uint64 GetSize();
	map<uint64, PlayerRef>* GetPlayersInRoom() { return &_players; }

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
	set<uint64> GetPartyIdsInRoom();
	void CreateParty(uint64 partyId);
	void RemoveParty(uint64 partyId);

	// TEMP
	void CheckPlayers();

private:
	//USE_LOCK;
	mutex _lock;
	map<uint64, PlayerRef> _players;
	vector <shared_ptr<RoomItem>> _roomItems;
	set<uint64> _partyIds;
};

extern Room GRoom;