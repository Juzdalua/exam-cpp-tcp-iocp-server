#pragma once
#include "Account.h"

class AccountController
{
public:
	static unique_ptr<Account> GetAccountById(uint64 accountId);
	static unique_ptr<Account> GetAccountByName(string accountName);
	static pair<shared_ptr<Account>, shared_ptr<Player>> GetAccountAndPlayerByName(string accountName);

	static bool CreateAccount(uint64 accountId, string name, string password);
};

