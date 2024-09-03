#pragma once

class Account
{
public:
	Account() = default;
	Account(uint64 id, const string& name)
		: _accountId(id), _name(name), _password("") {}
	Account(uint64 id, const string& name, const string& hashedPwd)
		: _accountId(id), _name(name), _password(hashedPwd) {}

public:
	uint64 GetAccountId() { return _accountId; };
	string GetAccountName() { return _name; };
	string GetHashedPwd() { return _password; };

private:
	uint64 _accountId;
	string _name;
	string _password;
};

