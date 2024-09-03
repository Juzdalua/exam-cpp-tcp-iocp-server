#include "pch.h"
#include "PlayerService.h"
#include <ConnectionPool.h>

unique_ptr<Player> PlayerService::GetPlayerByAccountId(uint64 accountId)
{
	string query = R"(
			SELECT
				id as playerId,
				account_id,
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
	cout << params[0] << endl;

	unique_ptr<sql::ResultSet> res = executeQuery(*CP, query, params);

	if (res->next())
	{
		uint64 playerId = res->getUInt64("playerId");
		uint64 accountId = res->getUInt64("accountId");
		float posX = res->getDouble("posX");
		float posY = res->getDouble("posY");
		float maxHP = res->getDouble("maxHP");
		float currentHP = res->getDouble("currentHP");

		return  make_unique<Player>(playerId, accountId, posX, posY, maxHP, currentHP);
	}
	return nullptr;
}
