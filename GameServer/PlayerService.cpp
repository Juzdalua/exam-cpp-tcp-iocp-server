#include "pch.h"
#include "PlayerService.h"
#include <ConnectionPool.h>

shared_ptr<Player> PlayerService::GetPlayerByAccountId(uint64 accountId)
{
	string query = R"(
			SELECT
				id as playerId,
				account_id,
				name as playerName,
				posX,
				posY,
				maxHP,
				currentHP,
				createdAt
			FROM
				player
			WHERE
				account_id = ?;
		)";
	vector<string>params;
	params.push_back(to_string(accountId));

	shared_ptr<sql::ResultSet> res = executeQuery(*CP, query, params);

	if (res->next())
	{
		uint64 playerId = res->getUInt64("playerId");
		uint64 accountId = res->getUInt64("accountId");
		string playerName = res->getString("playerName");
		float posX = static_cast<float>(res->getDouble("posX"));
		float posY = static_cast<float>(res->getDouble("posY"));
		uint64 maxHP = res->getUInt64("maxHP");
		uint64 currentHP = res->getUInt64("currentHP");

		return  make_shared<Player>(playerId, accountId, playerName, posX, posY, maxHP, currentHP);
	}
	return nullptr;
}

void PlayerService::UpdateMove(uint64 playerId, float posX, float posY)
{
	string query = R"(
		update player 
		set 
			posX = ?,
			posY = ?
			where id =?;
		)";

	vector<string>params;
	params.push_back(to_string(posX));
	params.push_back(to_string(posY));
	params.push_back(to_string(playerId));

	shared_ptr<sql::ResultSet> res = executeQuery(*CP, query, params);
}

uint64 PlayerService::DecreaseHP(uint64 playerId, uint64 damage)
{
	auto conn = CP->getConnection();
	if (!conn) {
		cerr << "Failed to get a connection from the pool." << endl;
		return -1;
	}

	try {
		conn->setAutoCommit(false);  

		string decreaseHPQuery = R"(
            update player
			set currentHP = if (currentHP <= ?, 0, currentHP - ?)
			where id = ?;
        )";
		shared_ptr<sql::PreparedStatement> pstmt(conn->prepareStatement(decreaseHPQuery));

		pstmt->setString(1, to_string(damage));
		pstmt->setString(2, to_string(damage));
		pstmt->setString(3, to_string(playerId));
		pstmt->executeUpdate();

		string getCurrentHPQuery = R"(
				SELECT currentHP FROM player WHERE id = ?
			)";
		shared_ptr<sql::PreparedStatement> pstmtPlayer(conn->prepareStatement(getCurrentHPQuery));
		pstmtPlayer->setString(1, to_string(playerId));
		shared_ptr<sql::ResultSet> res(pstmtPlayer->executeQuery());

		if (res->next())
		{
			uint64 currentHP = res->getUInt64(1);

			conn->commit();
			CP->releaseConnection(move(conn));
			return currentHP;
		}
		conn->rollback();
		CP->releaseConnection(move(conn));
		return -1;
	}
	catch (sql::SQLException& e) {
		conn->rollback();  
		CP->releaseConnection(move(conn));
		cerr << "SQLException: " << e.what() << endl;
		cerr << "Error code: " << e.getErrorCode() << endl;
		cerr << "SQL state: " << e.getSQLState() << endl;
		return -1;
	}
}

void PlayerService::UpdatePlayer(PlayerRef& player)
{
	string query = R"(
		update player 
		set 
			posX = ?,
			posY = ?,
			currentHP = ?
		where id =?;
		)";

	vector<string>params;
	params.push_back(to_string(player->GetPosX()));
	params.push_back(to_string(player->GetPosY()));
	params.push_back(to_string(player->GetCurrentHP()));
	params.push_back(to_string(player->GetPlayerId()));

	shared_ptr<sql::ResultSet> res = executeQuery(*CP, query, params);
}
