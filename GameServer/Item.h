#pragma once

enum ItemEffect
{
	HP,
	MP,
	ATK,
	DEF,
};

enum RoomItemState 
{
	AVAILABLE,
	RESPAWN_PENDING
};

class Item
{
public:
	Item() = default;
	Item(uint64 itemId, string itemName, string itemEffect, int64 value)
		:_itemId(itemId), _itemName(itemName), _itemEffect(itemEffect), _itemValue(value) {};
	~Item() {};

public:
	uint64 GetItemId() { return _itemId; }
	string GetItemName() { return _itemName; }
	string GetItemEffect() { return _itemEffect; }
	int64 GetItemValue() { return _itemValue; }

protected:
	uint64 _itemId;
	string _itemName;
	string _itemEffect;
	int64 _itemValue;
};

class RoomItem : public Item
{
public:
	RoomItem() = default;
	RoomItem(uint64 roomId, uint64 roomItemId, float posX, float posY, uint64 itemId, string itemName, string itemEffect, int64 value, RoomItemState state);
	~RoomItem() {};

public:
	uint64 GetRoomId() { return _roomId; }
	uint64 GetRoomItemId() { return _roomItemId; }
	float GetPosX() { return _posX; }
	float GetPosY() { return _posY; }
	RoomItemState GetRoomItemState() { return _state; }
	
	RoomItemState GetState() { return _state; }
	void SetState(RoomItemState state) { _state = state; }
	void EatItem();

private:
	uint64 _roomId;
	uint64 _roomItemId;
	float _posX;
	float _posY;

	RoomItemState _state = RoomItemState::AVAILABLE;
	chrono::steady_clock::time_point _respawnTime;
};
