#include "pch.h"
#include "AccountController.h"
#include "AccountService.h"

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
