#pragma once
#include "Player.h"

class PlayerService
{
public:
	static shared_ptr<Player> GetPlayerByAccountId(uint64 accountId);
	static void UpdateMove(uint64 playerId, float posX, float posY);
	static uint64 DecreaseHP(uint64 playerId, uint64 damage);
	static void UpdatePlayer(PlayerRef& player);
};

