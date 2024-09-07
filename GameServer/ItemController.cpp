#include "pch.h"
#include "ItemController.h"
#include "ItemService.h"

shared_ptr<Item> ItemController::GetItemById(uint64 id)
{
	return ItemService::GetItemById(id);
}

shared_ptr<Item> ItemController::GetItemByName(string name)
{
	return ItemService::GetItemByName(name);
}

vector<shared_ptr<RoomItem>> ItemController::GetRoomItemsByRoomId(uint64 roomId)
{
	return ItemService::GetRoomItemsByRoomId(roomId);
}

shared_ptr<RoomItem> ItemController::GetRoomItemByRoomItemId(uint64 roomItemId)
{
	return ItemService::GetRoomItemByRoomItemId(roomItemId);
}

void ItemController::InitRoomItems()
{
	return ItemService::InitRoomItems();
}

void ItemController::UpdateRoomItemByRoomItem(shared_ptr<RoomItem> roomItem)
{
	return ItemService::UpdateRoomItemByRoomItem(roomItem);
}
