#pragma once
#include "Player.h"

class PlayerService
{
public:
	static unique_ptr<Player> GetPlayerByAccountId(uint64 accountId);
};

