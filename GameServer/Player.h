#pragma once

enum PartyStatus
{
	PARTY_STATUS_AVAILABLE = 0,
	PARTY_STATUS_UNAVAILABLE = 1,
};

enum PartyPlayerStatus
{
	PARTY_PLAYER_STATUS_IN = 0,
	PARTY_PLAYER_STATUS_OUT = 0,
};

class Player
{
public:
	Player() = default;
	Player(uint64 id, uint64 accountId, string name, float posX, float posY, uint64 maxHP, uint64 currentHP)
		: _playerId(id), _accountId(accountId), _playerName(name), _posX(posX), _posY(posY), _maxHP(maxHP), _currentHP(currentHP) {}
	~Player() {};

	void SetOwnerSession(GameProtobufSessionRef session) { _ownerSession = session; }
	GameProtobufSessionRef GetOwnerSession() { return _ownerSession; }

	void SetPosition(float posX, float posY) { _posX = posX; _posY = posY; }
	
	void DecreaseHP(uint64 damage) { _currentHP -= damage; if (_currentHP < 0) _currentHP = 0; }
	void HealHP(uint64 heal) { _currentHP += heal; if (_currentHP > _maxHP) _currentHP = _maxHP; }
	void SetCurrentHP(uint64 currentHP) { _currentHP = currentHP; }

public:
	uint64 GetPlayerId() { return _playerId; }
	uint64 GetAccountId() { return _accountId; }
	string GetPlayerName() { return _playerName; }
	float GetPosX() { return _posX; }
	float GetPosY() { return _posY; }
	uint64 GetMaxHP() { return _maxHP; }
	uint64 GetCurrentHP() { return _currentHP; }

private:
	uint64 _playerId = 0;
	uint64 _accountId = 0;
	string _playerName;
	float _posX;
	float _posY;
	uint64 _maxHP;
	uint64 _currentHP;
	GameProtobufSessionRef _ownerSession = nullptr;
};

/*---------------
	Party
---------------*/
class Party
{
public:
	Party() = default;
	Party(uint64 partyId, PartyStatus status)
		: _partyId(partyId), _status(status) {};
	~Party() {};

public:
	uint64 GetPartyId() { return _partyId; }
	PartyStatus GetPartyStatus() { return _status; }

private:
	uint64 _partyId;
	PartyStatus _status;
};

class PartyPlayer
{
public:
	PartyPlayer() = default;
	PartyPlayer(uint64 partyId, uint64 playerId, PartyPlayerStatus status)
		:_partyId(partyId), _playerId(playerId), _status(status) {};
	~PartyPlayer() {};

public:
	uint64 GetPartyId() { return _partyId; }
	uint64 GetPlayerId() { return _playerId; }
	PartyPlayerStatus GetPartyPlayerStatus() { return _status; }

private:
	uint64 _partyId;
	uint64 _playerId;
	PartyPlayerStatus _status;
};