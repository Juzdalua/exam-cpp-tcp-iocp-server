#pragma once
#include "Item.h"

class ItemService
{
public:
	static unique_ptr<Item> GetItemById(uint64 id);
	static unique_ptr<Item> GetItemByName(string name);

	static vector<unique_ptr<RoomItem>> GetRoomItemsByRoomId(uint64 roomId);
	static unique_ptr<RoomItem> GetRoomItemByRoomItemId(uint64 roomItemId);
};

