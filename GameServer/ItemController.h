#pragma once
#include "Item.h"

class ItemController
{
public:
	static shared_ptr<Item> GetItemById(uint64 id);
	static shared_ptr<Item> GetItemByName(string name);

	static vector<shared_ptr<RoomItem>> GetRoomItemsByRoomId(uint64 roomId);
	static shared_ptr<RoomItem> GetRoomItemByRoomItemId(uint64 roomItemId);
};

