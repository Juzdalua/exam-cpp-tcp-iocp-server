#pragma once

class Account
{
public:
	Account() = default;
	Account(uint64 id, const string& name, const string& hashedPwd)
		: accountId(id), password(hashedPwd) {}

public:
	uint64 GetAccountId() { return accountId; };
	string GetHashedPwd() { return password; };

private:
	uint64 accountId;
	string name;
	string  password;
};

