#pragma once
#include "Account.h"

class AccountService
{
public:
	static unique_ptr<Account> GetAccountById(uint64 accountId);
	static unique_ptr<Account> GetAccountByName(string accountName);
	static unique_ptr<Account> GetAccountByPlayerId(uint64 playerId);
	
	static pair<shared_ptr<Account>, shared_ptr<Player>> GetAccountAndPlayerByName(string accountName);

	static bool CreateAccount(string name, string password);
	static bool CreateAccountAndPlayer(string name, string password);
};

