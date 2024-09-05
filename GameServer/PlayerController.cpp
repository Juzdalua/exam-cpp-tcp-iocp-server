#include "pch.h"
#include "PlayerController.h"
#include "PlayerService.h"

unique_ptr<Player> PlayerController::GetPlayerByAccountId(uint64 accountId)
{
    return PlayerService::GetPlayerByAccountId(accountId);
}

void PlayerController::UpdateMove(uint64 playerId, float posX, float posY)
{
    return PlayerService::UpdateMove(playerId, posX, posY);
}

uint64 PlayerController::DecreaseHP(uint64 playerId, float damage)
{
    return PlayerService::DecreaseHP(playerId, damage);
}
