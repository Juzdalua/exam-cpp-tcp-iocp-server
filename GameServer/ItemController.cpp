#include "pch.h"
#include "ItemController.h"
#include "ItemService.h"

unique_ptr<Item> ItemController::GetItemById(uint64 id)
{
	return ItemService::GetItemById(id);
}

unique_ptr<Item> ItemController::GetItemByName(string name)
{
	return ItemService::GetItemByName(name);
}

vector<unique_ptr<RoomItem>> ItemController::GetRoomItemsByRoomId(uint64 roomId)
{
	return ItemService::GetRoomItemsByRoomId(roomId);
}

unique_ptr<RoomItem> ItemController::GetRoomItemByRoomItemId(uint64 roomItemId)
{
	return ItemService::GetRoomItemByRoomItemId(roomItemId);
}
