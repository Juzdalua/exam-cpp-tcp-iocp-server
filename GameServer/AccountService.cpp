#include "pch.h"
#include "AccountService.h"
#include <ConnectionPool.h>
#include "Player.h"

shared_ptr<Account> AccountService::GetAccountById(uint64 accountId)
{
	string query = R"(
			SELECT
				id as accountId,
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

	unique_ptr<sql::ResultSet> res = executeQuery(*CP, query, params);

	if (res->next())
	{
		uint64 id = res->getUInt64("accountId");
		string name = res->getString("name");
		string hashedPwd = res->getString("password");

		return  make_shared<Account>(id, name, hashedPwd);
	}
	return nullptr;
}

shared_ptr<Account> AccountService::GetAccountByName(string accountName)
{
	string query = R"(
			SELECT
				id as accountId,
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

	unique_ptr<sql::ResultSet> res = executeQuery(*CP, query, params);

	if (res->next())
	{
		uint64 id = res->getUInt64("accountId");
		string name = res->getString("name");
		string hashedPwd = res->getString("password");

		return  make_shared<Account>(id, name, hashedPwd);
	}
	return nullptr;
}

shared_ptr<Account> AccountService::GetAccountByPlayerId(uint64 playerId)
{
	string query = R"(
			select 
				account.id as accountId,
				account.name 
			from
				account
			left join 
				player 
			on 
				player.account_id = account.id
			where 
				player.id = ?
		)";
	vector<string>params;
	params.push_back(to_string(playerId));

	unique_ptr<sql::ResultSet> res = executeQuery(*CP, query, params);

	if (res->next())
	{
		uint64 id = res->getUInt64("accountId");
		string name = res->getString("name");

		return  make_shared<Account>(id, name);
	}
	return nullptr;
}

pair<shared_ptr<Account>, shared_ptr<Player>> AccountService::GetAccountAndPlayerByName(string accountName)
{
	string query = R"(
			select
				account.id as accountId,
				account.name,
				account.password as password,
				player.id as playerId,
				player.name as playerName,
				player.posX,
				player.posY,
				player.maxHP,
				player.currentHP 
			from 
				account
			left join
				player 
			on
				account.id = player.account_id
			where 
				account.name = ?;
		)";
	vector<string>params;
	params.push_back(accountName);

	unique_ptr<sql::ResultSet> res = executeQuery(*CP, query, params);

	shared_ptr<Account> account = nullptr;
	shared_ptr<Player> player = nullptr;

	if (res->next())
	{
		uint64 accountId = res->getUInt64("accountId");
		string name = res->getString("name");
		string hashedPwd = res->getString("password");
		uint64 playerId = res->getUInt64("playerId");
		string playerName = res->getString("playerName");
		float posX = static_cast<float>(res->getDouble("posX"));
		float posY = static_cast<float>(res->getDouble("posY"));
		uint64 maxHP = res->getUInt64("maxHP");
		uint64 currentHP = res->getUInt64("currentHP");

		account = make_shared<Account>(accountId, name, hashedPwd);
		player = make_shared<Player>(playerId, accountId, playerName, posX, posY, maxHP, currentHP);

		return make_pair(account, player);
	}
	return make_pair(nullptr, nullptr);
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

bool AccountService::CreateAccountAndPlayer(string name, string password)
{
	auto conn = CP->getConnection();
	if (!conn) {
		cerr << "Failed to get a connection from the pool." << endl;
		return false;
	}

	try {
		conn->setAutoCommit(false);  // Æ®·£Àè¼Ç ½ÃÀÛ

		// °èÁ¤ »ðÀÔ Äõ¸®
		string accountInsertQuery = R"(
            INSERT INTO account (name, password) VALUES (?, ?);
        )";
		auto pstmt(conn->prepareStatement(accountInsertQuery));

		// °èÁ¤ »ðÀÔ Äõ¸® ÆÄ¶ó¹ÌÅÍ ¼³Á¤
		pstmt->setString(1, name);
		pstmt->setString(2, password);
		pstmt->executeUpdate();

		// ¸¶Áö¸· »ðÀÔµÈ ID °¡Á®¿À±â
		shared_ptr<sql::Statement> stmt(conn->createStatement());
		unique_ptr<sql::ResultSet> res(stmt->executeQuery("SELECT LAST_INSERT_ID()"));

		int insertId = 0;
		if (res->next()) {
			insertId = res->getInt(1);

			// ÇÃ·¹ÀÌ¾î »ðÀÔ Äõ¸®
			string playerInsertQuery = R"(
				INSERT INTO player (account_id, name) VALUES (?, ?);
			)";
			auto pstmtPlayer(conn->prepareStatement(playerInsertQuery));
			pstmtPlayer->setInt(1, insertId);
			pstmtPlayer->setString(2, name);
			pstmtPlayer->executeUpdate();

			conn->commit();  // Ä¿¹Ô
			CP->releaseConnection(move(conn));
			return true;
		}
		conn->rollback();
		CP->releaseConnection(move(conn));
		return false;
	}
	catch (sql::SQLException& e) {
		conn->rollback();  // ·Ñ¹é
		CP->releaseConnection(move(conn));
		cerr << "SQLException: " << e.what() << endl;
		cerr << "Error code: " << e.getErrorCode() << endl;
		cerr << "SQL state: " << e.getSQLState() << endl;
		return false;
	}
}

