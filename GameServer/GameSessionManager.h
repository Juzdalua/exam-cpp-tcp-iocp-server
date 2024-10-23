#pragma once

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

extern GameProtobufSessionManager GProtobufSessionManager;
