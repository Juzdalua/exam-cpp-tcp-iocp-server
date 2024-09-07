#include "pch.h"
#include "AccountController.h"
#include "AccountService.h"
#include "PlayerService.h"

shared_ptr<Account> AccountController::GetAccountById(uint64 accountId)
{
	shared_ptr<Account> account = AccountService::GetAccountById(accountId);
	if (account == nullptr)
		return nullptr;

	return account;
}

shared_ptr<Account> AccountController::GetAccountByName(string accountName)
{
	shared_ptr<Account> account = AccountService::GetAccountByName(accountName);
	if (account == nullptr)
		return nullptr;

	return account;
}

shared_ptr<Account> AccountController::GetAccountByPlayerId(uint64 playerId)
{
	shared_ptr<Account> account = AccountService::GetAccountByPlayerId(playerId);
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
	shared_ptr<Player> player = PlayerService::GetPlayerByAccountId(accountId);
	if (player == nullptr)
		return AccountService::CreateAccountAndPlayer(name, password);
	else
		return AccountService::CreateAccount(name, password);
}
