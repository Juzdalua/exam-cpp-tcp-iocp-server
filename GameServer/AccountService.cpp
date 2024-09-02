#include "pch.h"
#include "AccountService.h"
#include <ConnectionPool.h>

unique_ptr<Account> AccountService::GetAccountById(uint64 accountId)
{
	string query = R"(
			SELECT
				id,
				name,
				password,
				createdAt
			FROM
				account
			WHERE
				id = ?
		)";
	vector<string>params;
	params.push_back(to_string(accountId));
	cout << params[0] << endl;

	unique_ptr<sql::ResultSet> res = executeQuery(*CP, query, params);

	if (res->next())
	{
		uint64 id = res->getUInt64("id");
		string hashedPwd = res->getString("password");

		return  make_unique<Account>(id, hashedPwd);
	}
	return nullptr;
}

unique_ptr<Account> AccountService::GetAccountByName(string accountName)
{
	string query = R"(
			SELECT
				id,
				name,
				password,
				createdAt
			FROM
				account
			WHERE
				name = ?;
		)";
	vector<string>params;
	params.push_back(accountName);
	cout << params[0] << endl;

	unique_ptr<sql::ResultSet> res = executeQuery(*CP, query, params);

	if (res->next())
	{
		uint64 id = res->getUInt64("id");
		string hashedPwd = res->getString("password");

		return  make_unique<Account>(id, hashedPwd);
	}
	return nullptr;
}

bool AccountService::CreateAccount(string name, string password)
{
	try
	{
		string query = R"(
		INSERT INTO account (name, password) VALUES
			(?, ?);
	)";

		vector<string>params;
		params.push_back(name);
		params.push_back(password);

		unique_ptr<sql::ResultSet> res = executeQuery(*CP, query, params);

		return true;
	}
	catch (const std::exception&)
	{
		return false;
	}
}
