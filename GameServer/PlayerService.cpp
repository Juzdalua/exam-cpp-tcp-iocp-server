#include "pch.h"
#include "PlayerService.h"
#include <ConnectionPool.h>
#include "EnumMap.h"

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

int64 PlayerService::CreateParty(uint64 playerId)
{
	auto conn = CP->getConnection();
	if (!conn) {
		cerr << "Failed to get a connection from the pool." << endl;
		return -1;
	}

	try {
		conn->setAutoCommit(false);

		// Check isJoin player
		string isJoinPartyQuery = R"(
				select id 
				from partyplayer
				where status = "0"
				and playerId = ?;
			)";
		shared_ptr<sql::PreparedStatement> pstmtIsJoin(conn->prepareStatement(isJoinPartyQuery));
		pstmtIsJoin->setString(1, to_string(playerId));
		shared_ptr<sql::ResultSet> exRes(pstmtIsJoin->executeQuery());

		if (exRes->next())
		{
			conn->rollback();
			CP->releaseConnection(move(conn));
			return -1;
		}

		// Create Party
		string createPartyQuery = R"(
            insert into party() values ();
        )";
		shared_ptr<sql::PreparedStatement> pstmt(conn->prepareStatement(createPartyQuery));

		pstmt->executeUpdate();

		shared_ptr<sql::Statement> stmt(conn->createStatement());
		shared_ptr<sql::ResultSet> res(stmt->executeQuery("SELECT LAST_INSERT_ID()"));

		int insertId = 0;
		if (res->next()) {
			insertId = res->getInt(1);

			// ÇÃ·¹ÀÌ¾î »ðÀÔ Äõ¸®
			string partyPlayerInsertQuery = R"(
				insert into partyplayer (partyId, playerId) values(?, ?);
			)";
			shared_ptr<sql::PreparedStatement> pstmtPlayer(conn->prepareStatement(partyPlayerInsertQuery));
			pstmtPlayer->setInt(1, insertId);
			pstmtPlayer->setString(2, to_string(playerId));
			pstmtPlayer->executeUpdate();

			conn->commit();  // Ä¿¹Ô
			CP->releaseConnection(move(conn));
			return insertId;
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

int64 PlayerService::WithdrawParty(uint64 playerId, uint64 partyId)
{
	auto conn = CP->getConnection();
	if (!conn) {
		cerr << "Failed to get a connection from the pool." << endl;
		return -1;
	}

	try {
		conn->setAutoCommit(false);

		// Check Party
		string isAvailablePartyQuery = R"(
				select id 
				from party
				where status = "0"
				and id = ?;
			)";
		shared_ptr<sql::PreparedStatement> pstmtIsAvailable(conn->prepareStatement(isAvailablePartyQuery));
		pstmtIsAvailable->setString(1, to_string(partyId));
		shared_ptr<sql::ResultSet> checkPartyRes(pstmtIsAvailable->executeQuery());

		if (!checkPartyRes->next())
		{
			conn->rollback();
			CP->releaseConnection(move(conn));
			return -1;
		}

		// Check isJoin
		string isJoinPartyQuery = R"(
				select id
				from partyplayer
				where status = "0"
				and playerId = ?
				and partyId = ?;
			)";
		shared_ptr<sql::PreparedStatement> pstmtIsJoin(conn->prepareStatement(isJoinPartyQuery));
		pstmtIsJoin->setString(1, to_string(playerId));
		pstmtIsJoin->setString(2, to_string(partyId));
		shared_ptr<sql::ResultSet> isJoinRes(pstmtIsJoin->executeQuery());

		if (!isJoinRes->next())
		{
			conn->rollback();
			CP->releaseConnection(move(conn));
			return -1;
		}

		string withdrawPartyQuery = R"(
            update partyplayer 
			set status = "1"
			where playerId  = ?
			and partyId= ?;
        )";
		shared_ptr<sql::PreparedStatement> pstmt(conn->prepareStatement(withdrawPartyQuery));
		pstmt->setString(1, to_string(playerId));
		pstmt->setString(2, to_string(partyId));

		pstmt->executeUpdate();

		// Get Party Players Count
		string getCountQuery = R"(
            select count(id) as count
			from partyplayer
			where status = "0"
			and partyId = ?;
        )";

		shared_ptr<sql::PreparedStatement> pstmtGetCount(conn->prepareStatement(getCountQuery));
		pstmtGetCount->setString(1, to_string(partyId));
		shared_ptr<sql::ResultSet> getCountRes(pstmtGetCount->executeQuery());

		if (getCountRes->next()) {			
			conn->commit();  // Ä¿¹Ô
			CP->releaseConnection(move(conn));
			return getCountRes->getInt(1);
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

void PlayerService::CloseParty(uint64 partyId)
{
	string query = R"(
		update party 
		set 
			status = "1"
		where 
		id =?
		and status = "0";
		)";

	vector<string>params;
	params.push_back(to_string(partyId));

	shared_ptr<sql::ResultSet> res = executeQuery(*CP, query, params);
}

bool PlayerService::JoinParty(uint64 playerId, uint64 partyId)
{
	string query = R"(
		select
			if( (select count(id) from party where id = ? and status = "0") > 0, 
			if( (select count(id) from partyplayer where partyid = ? and status = "0") > 0, 
			if((select count(id) from partyplayer where playerid = ? and status = "0") = 0, 
			true, false), false ), false ) as canJoin;
		)";
	vector<string>params;
	params.push_back(to_string(partyId));
	params.push_back(to_string(partyId));
	params.push_back(to_string(playerId));

	shared_ptr<sql::ResultSet> res = executeQuery(*CP, query, params);

	if (res->next())
	{
		bool canJoin = res->getBoolean("canJoin");
		if (canJoin)
		{
			string query = R"(
		insert into partyplayer (partyId, playerId) values(? ,?);
		)";

			vector<string>params;
			params.push_back(to_string(partyId));
			params.push_back(to_string(playerId));

			shared_ptr<sql::ResultSet> res = executeQuery(*CP, query, params);
			return true;
		}
	}
	return false;
}

uint64 PlayerService::GetMyPartyIdByPlayerId(uint64 playerId)
{
	string query = R"(
			select partyId 
			from partyplayer 
			where 
			status = "0"
			and playerId = ?;
		)";
	vector<string>params;
	params.push_back(to_string(playerId));

	shared_ptr<sql::ResultSet> res = executeQuery(*CP, query, params);

	if (res->next())
	{
		cout << "Party Id: " << res->getUInt64("partyId") << endl;
		return res->getUInt64("partyId");
	}
	return 0;
}

vector<shared_ptr<Player>> PlayerService::GetPartyPlayersByPartyId(uint64 partyId)
{
	string query = R"(
			select 
				player.id as playerId,
				player.name as playerName,
				player.account_id as accountId,
				player.posX,
				player.posY ,
				player.maxHP ,
				player.currentHP
			from 
				party
			left join 
				partyplayer
			on
				party.id = partyplayer.partyId
			left join
				player
			on
				player.id = partyplayer.playerId 
			where 
				party.status = "0"
				and partyplayer.status  = "0"
				and party.id = ?
			order by partyplayer.createdAt asc;
		)";
	vector<string>params;
	params.push_back(to_string(partyId));

	shared_ptr<sql::ResultSet> res = executeQuery(*CP, query, params);

	vector<shared_ptr<Player>> players;
	while (res->next())
	{
		uint64 playerId = res->getUInt64("playerId");
		uint64 accountId = res->getUInt64("accountId");
		string playerName = res->getString("playerName");
		float posX = static_cast<float>(res->getDouble("posX"));
		float posY = static_cast<float>(res->getDouble("posY"));
		uint64 maxHP = res->getUInt64("maxHP");
		uint64 currentHP = res->getUInt64("currentHP");

		players.push_back(make_shared<Player>(playerId, accountId, playerName, posX, posY, maxHP, currentHP));
	}
	return players;
}

vector<shared_ptr<Player>> PlayerService::GetPartyPlayersByPlayerId(uint64 playerId)
{
	string query = R"(
			select 
				player.id as playerId,
				player.account_id as accountId,
				player.name as playerName,
				player.posX,
				player.posY ,
				player.maxHP ,
				player.currentHP
			from 
				party
			left join 
				partyplayer
			on
				party.id = partyplayer.partyId
			left join
				player
			on
				player.id = partyplayer.playerId 
			where 
				party.status = "0"
				and partyplayer.status  = "0"
				and party.id = (select partyId from partyplayer where playerId = ? and status = "0")
			order by partyplayer.createdAt asc;
		)";
	vector<string>params;
	params.push_back(to_string(playerId));

	shared_ptr<sql::ResultSet> res = executeQuery(*CP, query, params);

	vector<shared_ptr<Player>> players;
	while (res->next())
	{
		uint64 playerId = res->getUInt64("playerId");
		uint64 accountId = res->getUInt64("accountId");
		string playerName = res->getString("playerName");
		float posX = static_cast<float>(res->getDouble("posX"));
		float posY = static_cast<float>(res->getDouble("posY"));
		uint64 maxHP = res->getUInt64("maxHP");
		uint64 currentHP = res->getUInt64("currentHP");

		players.push_back(make_shared<Player>(playerId, accountId, playerName, posX, posY, maxHP, currentHP));
	}
	return players;
}

vector<pair<shared_ptr<Party>, shared_ptr<PartyPlayer>>> PlayerService::GetAllParties()
{
	string query = R"(
			select 
				party.id as partyId,
				party.status as partyStatus,
				partyplayer.playerId as playerId,
				partyplayer.status as partyplayerStatus
			from 
				party
			left join
				partyplayer 
			on
				party.id = partyplayer.partyId 
			where 
				party.status = "0"
				and partyplayer.status = "0"
			order by party.id asc;
		)";
	vector<string>params;

	shared_ptr<sql::ResultSet> res = executeQuery(*CP, query, params);

	vector<pair<shared_ptr<Party>, shared_ptr<PartyPlayer>>> parties;
	while (res->next())
	{
		uint64 partyId = res->getUInt64("partyId");
		string partyStatus = res->getString("partyStatus");

		uint64 playerId = res->getUInt64("playerId");
		string partyplayerStatus = res->getString("partyplayerStatus");

		shared_ptr<Party> party = make_shared<Party>(partyId, EnumMap::PartyStatusMap(partyStatus));
		shared_ptr<PartyPlayer> player = make_shared<PartyPlayer>(partyId, playerId, EnumMap::PartyPlayerStatusMap(partyplayerStatus));

		parties.push_back(make_pair(party, player));
	}
	return parties;
}
