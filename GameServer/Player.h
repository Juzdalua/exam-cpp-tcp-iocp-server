#pragma once

class Player
{
public:
	Player() = default;
	Player(uint64 id, uint64 accountId, float posX, float posY, float maxHP, float currentHP)
		: _playerId(id), _accountId(accountId), _posX(posX), _posY(posY), _maxHP(maxHP), _currentHP(currentHP) {}
	~Player() {};

	void SetOwnerSession(GameProtobufSessionRef session) { _ownerSession = session; }
	GameProtobufSessionRef GetOwnerSession() { return _ownerSession; }

public:
	uint64 GetPlayerId() { return _playerId; }
	uint64 GetAccountId() { return _accountId; }
	float GetPosX() { return _posX; }
	float GetPosY() { return _posY; }
	float GetMaxHP() { return _maxHP; }
	float GetCurrentHP() { return _currentHP; }

private:
	uint64 _playerId = 0;
	uint64 _accountId = 0;
	float _posX;
	float _posY;
	float _maxHP;
	float _currentHP;
	GameProtobufSessionRef _ownerSession;
};

