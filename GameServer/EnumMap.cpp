#include "pch.h"
#include "EnumMap.h"

Protocol::ItemType EnumMap::ItemTypeMap(string column)
{
	if (column == "HP")
		return Protocol::ITEM_TYPE_HEAL;
}

Protocol::ItemEffect EnumMap::ItemEffectMap(ItemEffect effect)
{
	switch (effect)
	{
	default:
	case HP:
		return Protocol::ITEM_EFFECT_HP;
		break;

	case MP:
		return Protocol::ITEM_EFFECT_MP;
		break;

	case ATK:
		return Protocol::ITEM_EFFECT_ATK;
		break;

	case DEF:
		return Protocol::ITEM_EFFECT_DEF;
		break;
	}
}

Protocol::RoomItemState EnumMap::RoomItemStateMap(RoomItemState state)
{
	switch (state)
	{
	default:
	case RoomItemState::AVAILABLE:
		return Protocol::ROOM_ITEM_STATE_AVAILABLE;
		break;

	case RoomItemState::RESPAWN_PENDING:
		return Protocol::ROOM_ITEM_STATE_RESPAWN_PENDING;
		break;
	}
}

PartyStatus EnumMap::PartyStatusMap(string column)
{
	unordered_map<string, PartyStatus> partyStatusColumnToEnum =
	{
		{"0", PartyStatus::PARTY_STATUS_AVAILABLE},
		{"1", PartyStatus::PARTY_STATUS_UNAVAILABLE},
	};

	return partyStatusColumnToEnum[column];
}

PartyPlayerStatus EnumMap::PartyPlayerStatusMap(string column)
{
	unordered_map<string, PartyPlayerStatus> partyPlayerStatusColumnToEnum =
	{
		{"0", PartyPlayerStatus::PARTY_PLAYER_STATUS_IN},
		{"1", PartyPlayerStatus::PARTY_PLAYER_STATUS_OUT},
	};

	return partyPlayerStatusColumnToEnum[column];
}