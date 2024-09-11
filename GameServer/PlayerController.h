#pragma once
#include "Player.h"

class PlayerController
{
public:
	static shared_ptr<Player> GetPlayerByAccountId(uint64 accountId);
	static void UpdateMove(uint64 playerId, float posX, float posY);
	static uint64 DecreaseHP(uint64 playerId, uint64 damage);
	static void UpdatePlayer(PlayerRef player);

	static int64 CreateParty(uint64 playerId);
	static int64 WithdrawParty(uint64 playerId, uint64 partyId);
	static void CloseParty(uint64 partyId);
	static bool JoinParty(uint64 playerId, uint64 partyId);

	static uint64 GetMyPartyIdByPlayerId(uint64 playerId);
	static vector<shared_ptr<Player>> GetPartyPlayersByPartyId(uint64 partyId);
	static vector<shared_ptr<Player>> GetPartyPlayersByPlayerId(uint64 playerId);
	static vector<pair<shared_ptr<Party>, shared_ptr<PartyPlayer>>> GetAllParties();
};

