#pragma once
class Account
{
public:
	Account();
	Account(uint64 id, const std::string& hashedPwd)
		: _id(id), password(hashedPwd) {}

public:
	uint64 GetAccountId() { return _id; };
	string GetHashedPwd() { return password; };

private:
	uint64 _id;
	string  password;
};

