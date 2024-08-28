#include "pch.h"
#include "GameSessionManager.h"
#include "GameSession.h"

GameSessionManager GSessionManager;

void GameSessionManager::Add(GameSessionRef session)
{
	//WRITE_LOCK;
	lock_guard<mutex> lock(_lock);
	_sessions.insert(session);
}

void GameSessionManager::Remove(GameSessionRef session)
{
	//WRITE_LOCK;
	lock_guard<mutex> lock(_lock);
	_sessions.erase(session);
}

void GameSessionManager::Broadcast(SendBufferRef sendBuffer)
{
	//WRITE_LOCK;
	lock_guard<mutex> lock(_lock);
	for (GameSessionRef session : _sessions)
	{
		session->Send(sendBuffer);
	}
}