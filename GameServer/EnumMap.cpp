#include "pch.h"
#include "EnumMap.h"

Protocol::ItemType EnumMap::ItemTypeProtocolMap(ItemEffect effect)
{
	switch (effect)
	{
	default:
	case HP:
		return Protocol::ITEM_TYPE_HEAL;
		break;
	case MP:
		// TODO
		return Protocol::ITEM_TYPE_HEAL;
		break;
	case ATK:
		// TODO
		return Protocol::ITEM_TYPE_HEAL;
		break;
	case DEF:
		// TODO
		return Protocol::ITEM_TYPE_HEAL;
		break;	
	}
}

Protocol::ItemEffect EnumMap::ItemEffectProtocolMap(ItemEffect effect)
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

Protocol::RoomItemState EnumMap::RoomItemStateProtocolMap(RoomItemState state)
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

ItemEffect EnumMap::ItemEffectMap(string column)
{
	unordered_map<string, ItemEffect> itemEffectColumnToEnum =
	{
		{"0", ItemEffect::HP},
		{"1", ItemEffect::MP},
		{"2", ItemEffect::ATK},
		{"3", ItemEffect::DEF},
	};

	auto it = itemEffectColumnToEnum.find(column);
	if (it != itemEffectColumnToEnum.end())
	{
		return it->second;
	}
	else
	{
		// ó������ ���� ���� ���� ���� ó��
		cerr << "Invalid column value: " << column << endl;
		return ItemEffect::HP;  // �⺻�� ��ȯ �Ǵ� ���� ó��
	}
}

RoomItemState EnumMap::RoomItemStateMap(string column)
{
	unordered_map<string, RoomItemState> roomItemStateColumnToEnum =
	{
		{"0", RoomItemState::AVAILABLE},
		{"1", RoomItemState::RESPAWN_PENDING},
	};

	auto it = roomItemStateColumnToEnum.find(column);
	if (it != roomItemStateColumnToEnum.end())
	{
		return it->second;
	}
	else
	{
		// ó������ ���� ���� ���� ���� ó��
		cerr << "Invalid column value: " << column << endl;
		return RoomItemState::AVAILABLE;  // �⺻�� ��ȯ �Ǵ� ���� ó��
	}
}

PartyStatus EnumMap::PartyStatusMap(string column)
{
	unordered_map<string, PartyStatus> partyStatusColumnToEnum =
	{
		{"0", PartyStatus::PARTY_STATUS_AVAILABLE},
		{"1", PartyStatus::PARTY_STATUS_UNAVAILABLE},
	};

	//return partyStatusColumnToEnum[column];

	auto it = partyStatusColumnToEnum.find(column);
	if (it != partyStatusColumnToEnum.end())
	{
		return it->second;
	}
	else
	{
		// ó������ ���� ���� ���� ���� ó��
		cerr << "Invalid column value: " << column << endl;
		return PartyStatus::PARTY_STATUS_UNAVAILABLE;  // �⺻�� ��ȯ �Ǵ� ���� ó��
	}
}

PartyPlayerStatus EnumMap::PartyPlayerStatusMap(string column)
{
	unordered_map<string, PartyPlayerStatus> partyPlayerStatusColumnToEnum =
	{
		{"0", PartyPlayerStatus::PARTY_PLAYER_STATUS_IN},
		{"1", PartyPlayerStatus::PARTY_PLAYER_STATUS_OUT},
	};

	//return partyPlayerStatusColumnToEnum[column];

	auto it = partyPlayerStatusColumnToEnum.find(column);
	if (it != partyPlayerStatusColumnToEnum.end())
	{
		return it->second;
	}
	else
	{
		// ó������ ���� ���� ���� ���� ó��
		cerr << "Invalid column value: " << column << endl;
		return PartyPlayerStatus::PARTY_PLAYER_STATUS_IN;  // �⺻�� ��ȯ �Ǵ� ���� ó��
	}
}
