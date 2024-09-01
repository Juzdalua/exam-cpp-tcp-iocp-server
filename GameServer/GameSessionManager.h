#pragma once

class GameSession;

using GameSessionRef = shared_ptr<GameSession>;
using GamePacketSessionRef = shared_ptr<GamePacketSession>;

/*---------------
	Game Session
---------------*/
class GameSessionManager
{
public:
	void Add(GameSessionRef session);
	void Remove(GameSessionRef session);
	void Broadcast(SendBufferRef sendBuffer);

private:
	//USE_LOCK;
	mutex _lock;
	set<GameSessionRef> _sessions;
};

/*----------------------
	Game Packet Session
----------------------*/
class GamePacketSessionManager
{
public:
	void Add(GamePacketSessionRef session);
	void Remove(GamePacketSessionRef session);
	void Broadcast(SendBufferRef sendBuffer);

private:
	//USE_LOCK;
	mutex _lock;
	set<GamePacketSessionRef> _sessions;
};

/*----------------------
	Game Protobuf Session
----------------------*/
class GameProtobufSessionManager
{
public:
	void Add(GameProtobufSessionRef session);
	void Remove(GameProtobufSessionRef session);
	void Broadcast(SendBufferRef sendBuffer);

private:
	//USE_LOCK;
	mutex _lock;
	set<GameProtobufSessionRef> _sessions;
};

extern GameSessionManager GSessionManager;
extern GamePacketSessionManager GPacketSessionManager;
extern GameProtobufSessionManager GProtobufSessionManager;
