#pragma once
#include "Item.h"
#include "Player.h"
#include "Job.h"
#include "Protocol.pb.h"

/*--------------
	Room
--------------*/
class Room
{
	friend class EnterRoomJob;
	friend class LeaveRoomJob;
	friend class BroadcastRoomJob;
	friend class SendToRoomTargetPlayerJob;
	friend class SendWithSessionJob;

	friend class IsLoginRoomPlayerJob;
	friend class CreateRoomJob;

	friend class MoveRoomJob;
	friend class HitRoomJob;

	friend class EatItemRoomJob;
	friend class UpdateItemRoomJob;

public:
	Room();
	~Room();

private:
	void Enter(PlayerRef player);
	void Leave(PlayerRef player);
	void Broadcast(SendBufferRef sendBuffer);
	void SendToTargetPlayer(uint64 targetPlayerId, SendBufferRef sendBuffer);

	bool IsLogin(uint64 playerId);

	uint64 GetSize();
	map<uint64, PlayerRef>* GetPlayersInRoom() { return &_players; }

	// Move
	bool CanGo(uint64 playerId, float posX, float posY);
	void UpdateMove(uint64 playerId, float posX, float posY);

	// HP
	void UpdateCurrentHP(uint64 playerId, uint64 currentHP);

	// Room Item
	vector <shared_ptr<RoomItem>> GetRoomItems() { return _roomItems; }
	void SetRoomItems(const vector<shared_ptr<RoomItem>>& roomItems);
	void UpdateRoomItem(const shared_ptr<RoomItem>& roomItem);

public:
	void CheckPlayers();

public:
	void PushJob(JobRef job) { _jobs.Push(job); }
	void FlushJob();

private:
	map<uint64, PlayerRef> _players;
	vector <shared_ptr<RoomItem>> _roomItems;
	JobQueue _jobs;
	atomic<bool> _isExecute = false;
	mutex _lock;
};

extern Room GRoom;

/*--------------
	Room Job
--------------*/
class EnterRoomJob : public IJob
{
public:
	EnterRoomJob(Room& room, PlayerRef player)
		:_room(room), _player(player) {}

	virtual void Execute() override;

public:
	Room& _room;
	PlayerRef _player;
};

class LeaveRoomJob : public IJob
{
public:
	LeaveRoomJob(Room& room, PlayerRef player)
		:_room(room), _player(player) {}

	virtual void Execute() override;	

public:
	Room& _room;
	PlayerRef _player;
};

class BroadcastRoomJob : public IJob
{
public:
	BroadcastRoomJob(Room& room, SendBufferRef sendBuffer)
		:_room(room), _sendBuffer(sendBuffer) {}

	virtual void Execute() override;
	
public:
	Room& _room;
	SendBufferRef _sendBuffer;
};

class SendWithSessionJob : public IJob
{
public:
	SendWithSessionJob(Room& room, GameProtobufSessionRef session, SendBufferRef sendBuffer)
		:_room(room), _session(session), _sendBuffer(sendBuffer) {}

	virtual void Execute() override;

public:
	Room& _room;
	GameProtobufSessionRef _session;
	SendBufferRef _sendBuffer;
};

class SendToRoomTargetPlayerJob : public IJob
{
public:
	SendToRoomTargetPlayerJob(Room& room, uint64 targetPlayerId, SendBufferRef sendBuffer)
		:_room(room), _targetPlayerId(targetPlayerId), _sendBuffer(sendBuffer) {}

	virtual void Execute() override;

public:
	Room& _room;
	SendBufferRef _sendBuffer;
	uint64 _targetPlayerId;
};

class IsLoginRoomPlayerJob : public IJob
{
public:
	IsLoginRoomPlayerJob(Room& room, PlayerRef player, GameProtobufSessionRef& session)
		:_room(room), _player(player), _session(session) {}

	virtual void Execute() override;

public:
	Room& _room;
	SendBufferRef _sendBuffer;
	PlayerRef _player;
	GameProtobufSessionRef _session;
};

class CreateRoomJob : public IJob
{
public:
	CreateRoomJob(Room& room, uint64 targetPlayerId)
		:_room(room), _targetPlayerId(targetPlayerId) {}

	virtual void Execute() override;

public:
	Room& _room;
	uint64 _targetPlayerId;
};

class MoveRoomJob : public IJob
{
public:
	MoveRoomJob(Room& room, PlayerRef player, Protocol::C_MOVE recvPkt)
		:_room(room), _player(player), _recvPkt(recvPkt) {}

	virtual void Execute() override;

public:
	Room& _room;
	PlayerRef _player;
	Protocol::C_MOVE _recvPkt;
};

class HitRoomJob : public IJob
{
public:
	HitRoomJob(Room& room, PlayerRef player, uint64 damage, uint64 currentHP)
		:_room(room), _player(player), _damage(damage), _currentHP(currentHP) {}

	virtual void Execute() override;

public:
	Room& _room;
	PlayerRef _player;
	uint64 _damage;
	uint64 _currentHP;
};

class UpdateItemRoomJob : public IJob
{
public:
	UpdateItemRoomJob(Room& room, shared_ptr<RoomItem> roomItem, RoomItemState status)
		:_room(room), _roomItem(roomItem), _status(status) {}

	virtual void Execute() override;

public:
	Room& _room;
	shared_ptr<RoomItem> _roomItem;
	RoomItemState _status;
};

class EatItemRoomJob : public IJob
{
public:
	EatItemRoomJob(Room& room, PlayerRef player, shared_ptr<RoomItem> roomItem, Protocol::ItemType itemType)
		:_room(room), _player(player), _roomItem(roomItem), _itemType(itemType) {}

	virtual void Execute() override;

public:
	Room& _room;
	PlayerRef _player;
	shared_ptr<RoomItem> _roomItem;
	Protocol::ItemType _itemType;
};
