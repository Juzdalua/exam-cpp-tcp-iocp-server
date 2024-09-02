#pragma once
#include "Account.h"

class AccountService
{
public:
	static unique_ptr<Account> GetAccountById(uint64 accountId);
	static unique_ptr<Account> GetAccountByName(string accountName);
	static pair<shared_ptr<Account>, shared_ptr<Player>> GetAccountAndPlayerByName1(string accountName);

	static bool CreateAccount(string name, string password);
	static bool CreateAccountAndPlayer(string name, string password);
};

