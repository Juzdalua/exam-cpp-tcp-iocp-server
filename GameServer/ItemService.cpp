#include "pch.h"
#include "ItemService.h"
#include <ConnectionPool.h>
#include "EnumMap.h"

shared_ptr<Item> ItemService::GetItemById(uint64 id)
{
	string query = R"(
			select 
				id as itemId,
				name as itemName,
				effect as itemEffect,
				value as itemValue
			from item 
			where id = ?;
		)";
	vector<string>params;
	params.push_back(to_string(id));

	shared_ptr<sql::ResultSet> res = executeQuery(*CP, query, params);

	if (res->next())
	{
		uint64 id = res->getUInt64("itemId");
		string name = res->getString("itemName");
		string effect = res->getString("itemEffect");
		int value = res->getInt("itemValue");
		cout << "ID: " << id << endl;
		cout << "name: " << name << endl;
		cout << "effect: " << effect << endl;
		cout << "value: " << value << endl;
		
		return  make_shared<Item>(id, name, EnumMap::ItemEffectMap(effect), value);
	}
	return nullptr;
}

shared_ptr<Item> ItemService::GetItemByName(string name)
{
	string query = R"(
			select 
				id as itemId,
				name as itemName,
				effect as itemEffect,
				value as itemValue
			from item 
			where name = ?;
		)";
	vector<string>params;
	params.push_back(name);

	shared_ptr<sql::ResultSet> res = executeQuery(*CP, query, params);

	if (res->next())
	{
		uint64 id = res->getUInt64("itemId");
		string name = res->getString("itemName");
		string effect = res->getString("itemEffect");
		int value= res->getInt("itemValue");

		return  make_shared<Item>(id, name, EnumMap::ItemEffectMap(effect), value);
	}
	return nullptr;
}

vector<shared_ptr<RoomItem>> ItemService::GetRoomItemsByRoomId(uint64 roomId)
{
	string query = R"(
			select 
				roomitem.roomId as roomId,
				roomitem.id as roomItemId,
				item.id as itemId,
				item.name as itemName,
				item.effect as itemEffect,
				item.value as itemValue,
				roomitem.posX,
				roomitem.posY ,
				roomitem.state
			from 
				item
			left join
				roomitem
			on
				item.id = roomitem.itemId 
			where 
				roomId = ?
			order by roomitem.id asc;
		)";
	vector<string>params;
	params.push_back(to_string(roomId));

	shared_ptr<sql::ResultSet> res = executeQuery(*CP, query, params);

	vector<shared_ptr<RoomItem>> roomItems;
	while (res->next())
	{
		uint64 roomId = res->getUInt64("roomId");
		uint64 roomItemId = res->getUInt64("roomItemId");
		uint64 itemId = res->getUInt64("itemId");
		string itemName = res->getString("itemName");
		string itemEffect = res->getString("itemEffect");
		int itemValue = res->getInt("itemValue");
		float posX = static_cast<float>(res->getDouble("posX"));
		float posY = static_cast<float>(res->getDouble("posY"));
		string state = res->getString("state");

		roomItems.push_back(make_shared<RoomItem>(roomId, roomItemId, posX, posY, itemId, itemName, EnumMap::ItemEffectMap(itemEffect), itemValue, EnumMap::RoomItemStateMap(state)));
	}
	return roomItems;
}

shared_ptr<RoomItem> ItemService::GetRoomItemByRoomItemId(uint64 roomItemId)
{
	string query = R"(
			select 
				roomitem.roomId as roomId,
				roomitem.id as roomItemId,
				item.id as itemId,
				item.name as itemName,
				item.effect as itemEffect,
				item.value as itemValue,
				roomitem.posX,
				roomitem.posY,
				roomitem.state
			from 
				item
			left join
				roomitem
			on
				item.id = roomitem.itemId 
			where 
				roomitem.id = ?
			order by roomitem.id asc;
		)";
	vector<string>params;
	params.push_back(to_string(roomItemId));

	shared_ptr<sql::ResultSet> res = executeQuery(*CP, query, params);

	if (res->next())
	{
		uint64 roomId = res->getUInt64("roomId");
		uint64 roomItemId = res->getUInt64("roomItemId");
		uint64 itemId = res->getUInt64("itemId");
		string itemName = res->getString("itemName");
		string itemEffect = res->getString("itemEffect");
		int itemValue = res->getInt("itemValue");
		float posX = static_cast<float>(res->getDouble("posX"));
		float posY = static_cast<float>(res->getDouble("posY"));
		string state = res->getString("state");

		return make_shared<RoomItem>(roomId, roomItemId, posX, posY, itemId, itemName, EnumMap::ItemEffectMap(itemEffect), itemValue, EnumMap::RoomItemStateMap(state));
	}
	return nullptr;
}

void ItemService::InitRoomItems()
{
	string query = R"(
		update roomitem 
		set 
		state = "0"
		)";

	vector<string>params;

	shared_ptr<sql::ResultSet> res = executeQuery(*CP, query, params);
}

void ItemService::UpdateRoomItemByRoomItem(shared_ptr<RoomItem> roomItem)
{
	string query = R"(
		update roomitem 
		set 
		posX = ?,
		posY = ?,
		state = ?
		where id = ?;
		)";

	vector<string>params;
	params.push_back(to_string(roomItem->GetPosX()));
	params.push_back(to_string(roomItem->GetPosY()));
	params.push_back(to_string(roomItem->GetState()));
	params.push_back(to_string(roomItem->GetRoomItemId()));

	shared_ptr<sql::ResultSet> res = executeQuery(*CP, query, params);
}
