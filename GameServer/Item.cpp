#include "pch.h"
#include "Item.h"

RoomItem::RoomItem(uint64 roomId, uint64 roomItemId, float posX, float posY, uint64 itemId, string itemName, ItemEffect itemEffect, int64 value, RoomItemState state)
	:Item(itemId, itemName, itemEffect, value),
	_roomId(roomId), _roomItemId(roomItemId), _posX(posX), _posY(posY), _state(state)
{
}

void RoomItem::EatItem()
{
	_state = RoomItemState::RESPAWN_PENDING;
	_respawnTime = chrono::steady_clock::now() + chrono::seconds(5);
}
