#include "pch.h"
#include "PlayerController.h"
#include "PlayerService.h"

shared_ptr<Player> PlayerController::GetPlayerByAccountId(uint64 accountId)
{
    return PlayerService::GetPlayerByAccountId(accountId);
}

void PlayerController::UpdateMove(uint64 playerId, float posX, float posY)
{
    return PlayerService::UpdateMove(playerId, posX, posY);
}

uint64 PlayerController::DecreaseHP(uint64 playerId, uint64 damage)
{
    return PlayerService::DecreaseHP(playerId, damage);
}

void PlayerController::UpdatePlayer(PlayerRef& player)
{
    return PlayerService::UpdatePlayer(player);
}

int64 PlayerController::CreateParty(uint64 playerId)
{
    return PlayerService::CreateParty(playerId);
}

int64 PlayerController::WithdrawParty(uint64 playerId, uint64 partyId)
{
    return PlayerService::WithdrawParty(playerId, partyId);
}

void PlayerController::CloseParty(uint64 partyId)
{
    return PlayerService::CloseParty(partyId);
}

uint64 PlayerController::GetMyPartyIdByPlayerId(uint64 playerId)
{
    return PlayerService::GetMyPartyIdByPlayerId(playerId);
}

vector<shared_ptr<Player>> PlayerController::GetPartyPlayersByPartyId(uint64 partyId)
{
    return PlayerService::GetPartyPlayersByPartyId(partyId);
}

vector<shared_ptr<Player>> PlayerController::GetPartyPlayersByPlayerId(uint64 playerId)
{
    return PlayerService::GetPartyPlayersByPlayerId(playerId);
}

vector<pair<shared_ptr<Party>, shared_ptr<PartyPlayer>>> PlayerController::GetAllParties()
{
    return PlayerService::GetAllParties();
}
