#pragma once

class Player
{
public:
	Player() = default;
	Player(uint64 id, uint64 accountId, string posX, string posY, float maxHP, float currentHP)
		: _playerId(id), _accountId(accountId), _posX(posX), _posY(posY), _maxHP(maxHP), _currentHP(currentHP) {}
	~Player() {};

public:
	uint64 GetPlayerId() { return _playerId; }
	uint64 GetAccountId() { return _accountId; }
	string GetPosX() { return _posX; }
	string GetPosY() { return _posY; }
	float GetMaxHP() { return _maxHP; }
	float GetCurrentHP() { return _currentHP; }

private:
	uint64 _playerId = 0;
	uint64 _accountId = 0;
	string _posX;
	string _posY;
	float _maxHP;
	float _currentHP;
	GameSessionRef _ownerSession;
};

