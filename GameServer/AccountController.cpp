#include "pch.h"
#include "AccountController.h"
#include "AccountService.h"
#include "PlayerService.h"

unique_ptr<Account> AccountController::GetAccountById(uint64 accountId)
{
	unique_ptr<Account> account = AccountService::GetAccountById(accountId);
	if (account == nullptr)
		return nullptr;

	return account;
}

unique_ptr<Account> AccountController::GetAccountByName(string accountName)
{
	unique_ptr<Account> account = AccountService::GetAccountByName(accountName);
	if (account == nullptr)
		return nullptr;

	return account;
}

unique_ptr<Account> AccountController::GetAccountByPlayerId(uint64 playerId)
{
	unique_ptr<Account> account = AccountService::GetAccountByPlayerId(playerId);
	if (account == nullptr)
		return nullptr;

	return account;
}

pair<shared_ptr<Account>, shared_ptr<Player>> AccountController::GetAccountAndPlayerByName(string accountName)
{
	pair<shared_ptr<Account>, shared_ptr<Player>> pariAccountPlayer = AccountService::GetAccountAndPlayerByName(accountName);
	if (pariAccountPlayer.first == nullptr)
		return make_pair(nullptr, nullptr);

	return pariAccountPlayer;
}

bool AccountController::CreateAccount(uint64 accountId, string name, string password)
{
	unique_ptr<Player> player = PlayerService::GetPlayerByAccountId(accountId);
	if (player == nullptr)
		return AccountService::CreateAccountAndPlayer(name, password);
	else
		return AccountService::CreateAccount(name, password);
}
