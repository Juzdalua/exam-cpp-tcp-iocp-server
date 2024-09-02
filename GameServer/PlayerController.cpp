#include "pch.h"
#include "PlayerController.h"
#include "PlayerService.h"

unique_ptr<Player> PlayerController::GetPlayerByAccountId(uint64 accountId)
{
    return PlayerService::GetPlayerByAccountId(accountId);
}
