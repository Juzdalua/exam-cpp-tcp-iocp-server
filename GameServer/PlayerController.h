#pragma once
#include "Player.h"

class PlayerController
{
public:
	static unique_ptr<Player> GetPlayerByAccountId(uint64 accountId);
	static void UpdateMove(uint64 playerId, float posX, float posY);
	static uint64 DecreaseHP(uint64 playerId, float damage);
};

