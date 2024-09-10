#pragma once
#include "Protocol.pb.h"
#include "Item.h"
#include "Player.h"

class EnumMap
{
public:
	static Protocol::ItemType ItemTypeProtocolMap(ItemEffect effect);
	static Protocol::ItemEffect ItemEffectProtocolMap(ItemEffect effect);
	static Protocol::RoomItemState RoomItemStateProtocolMap(RoomItemState state);
	
	static ItemEffect ItemEffectMap(string column);
	static RoomItemState RoomItemStateMap(string column);

	static PartyStatus PartyStatusMap(string column);
	static PartyPlayerStatus PartyPlayerStatusMap(string column);
};

