#pragma once
#include "Player.h"

class PlayerController
{
public:
	static unique_ptr<Player> GetPlayerByAccountId(uint64 accountId);
};

