#include "pch.h"
#include "GameSessionManager.h"
#include "GameSession.h"

GameProtobufSessionManager GProtobufSessionManager;

/*----------------------
	Game Protobuf Session
----------------------*/
void GameProtobufSessionManager::Add(GameProtobufSessionRef session)
{
	//WRITE_LOCK;
	lock_guard<mutex> lock(_lock);
	_sessions.insert(session);
}

void GameProtobufSessionManager::Remove(GameProtobufSessionRef session)
{
	//WRITE_LOCK;
	lock_guard<mutex> lock(_lock);
	_sessions.erase(session);
}

void GameProtobufSessionManager::Broadcast(SendBufferRef sendBuffer)
{
	//WRITE_LOCK;
	lock_guard<mutex> lock(_lock);
	for (GameProtobufSessionRef session : _sessions)
	{
		session->Send(sendBuffer);
	}
}