#pragma once
#include "Protocol.pb.h"
#include "Item.h"

class EnumMap
{
	static Protocol::ItemType ItemTypeMap(string column);
	static Protocol::ItemEffect ItemEffectMap(ItemEffect effect);
	static Protocol::RoomItemState RoomItemStateMap(RoomItemState state);
};

