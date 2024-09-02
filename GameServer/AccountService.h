#pragma once
#include "Account.h"

class AccountService
{
public:
	static unique_ptr<Account> GetAccountById(uint64 accountId);
	static unique_ptr<Account> GetAccountByName(string accountName);

	static bool CreateAccount(string name, string password);
};

