#include "pch.h"
#include "Item.h"

void RoomItem::EatItem()
{
	_state = RoomItemState::RESPAWN_PENDING;
	_respawnTime = chrono::steady_clock::now() + chrono::seconds(5);
}
