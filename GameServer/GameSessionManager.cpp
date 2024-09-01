#include "pch.h"
#include "GameSessionManager.h"
#include "GameSession.h"

GameSessionManager GSessionManager;
GamePacketSessionManager GPacketSessionManager;
GameProtobufSessionManager GProtobufSessionManager;

/*---------------
	Game Session
---------------*/
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

/*----------------------
	Game Packet Session
----------------------*/
void GamePacketSessionManager::Add(GamePacketSessionRef session)
{
	//WRITE_LOCK;
	lock_guard<mutex> lock(_lock);
	_sessions.insert(session);
}

void GamePacketSessionManager::Remove(GamePacketSessionRef session)
{
	//WRITE_LOCK;
	lock_guard<mutex> lock(_lock);
	_sessions.erase(session);
}

void GamePacketSessionManager::Broadcast(SendBufferRef sendBuffer)
{
	//WRITE_LOCK;
	lock_guard<mutex> lock(_lock);
	for (GamePacketSessionRef session : _sessions)
	{
		session->Send(sendBuffer);
	}
}

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