#pragma once
#include "Protocol.pb.h"
#include "Item.h"
#include "Player.h"

class EnumMap
{
public:
	static Protocol::ItemType ItemTypeMap(string column);
	static Protocol::ItemEffect ItemEffectMap(ItemEffect effect);
	static Protocol::RoomItemState RoomItemStateMap(RoomItemState state);
	
	static PartyStatus PartyStatusMap(string column);
	static PartyPlayerStatus PartyPlayerStatusMap(string column);
};

