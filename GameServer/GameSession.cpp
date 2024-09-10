#pragma once
#include "pch.h"
#include "GameSession.h"
#include "GameSessionManager.h"
#include "Protocol.pb.h"
#include "Room.h"
#include "Player.h"
#include "ClientPacketHandler.h"
#include "PacketUtils.h"

/*---------------
	Game Session
	Without Header
---------------*/
void GameSession::OnConnected()
{
	GSessionManager.Add(static_pointer_cast<GameSession>(shared_from_this()));
}

void GameSession::OnDisconnected()
{
	GSessionManager.Remove(static_pointer_cast<GameSession>(shared_from_this()));
}

int32 GameSession::OnRecv(BYTE* buffer, int32 len)
{
	// Echo
	cout << "OnRecv Len = " << len << endl;
	cout << "OnRecv Data = " << buffer << endl;

	SendBufferRef sendBuffer = SendBufferRef(new SendBuffer(4096));
	sendBuffer->CopyData(buffer, len);

	GSessionManager.Broadcast(sendBuffer);

	return len;
}

void GameSession::OnSend(int32 len)
{
	cout << "OnSend Len = " << len << endl;
}

/*----------------------
	Game Packet Session
	With PacketHeader
----------------------*/
void GamePacketSession::OnConnected()
{
	GPacketSessionManager.Add(static_pointer_cast<GamePacketSession>(shared_from_this()));
}

void GamePacketSession::OnDisconnected()
{
	GPacketSessionManager.Remove(static_pointer_cast<GamePacketSession>(shared_from_this()));
}

void GamePacketSession::OnRecvPacket(BYTE* buffer, int32 len)
{
	PacketSessionRef session = GetPacketSessionRef();
	PacketHeader* header = reinterpret_cast<PacketHeader*>(buffer);

	cout << "Packet Id: " << header->id << ", SIze: " << header->size << endl;
	cout << &buffer[4] << endl;

	SendBufferRef sendBuffer = SendBufferRef(new SendBuffer(4096));
	reinterpret_cast<PacketHeader*>(sendBuffer->Buffer())->id = reinterpret_cast<PacketHeader*>(buffer)->id;
	reinterpret_cast<PacketHeader*>(sendBuffer->Buffer())->size = len;
	sendBuffer->CopyPacket(&sendBuffer->Buffer()[4], buffer, header->size - sizeof(PacketHeader));

	GPacketSessionManager.Broadcast(sendBuffer);

	// TODO: PacketId 대역 체크
	//ClientPacketHandler::HandlePacket(session, buffer, len);
}

void GamePacketSession::OnSend(int32 len)
{

}

/*-------------------
	Game Protobuf Session
	With PakcetHeader
-------------------*/
void GameProtobufSession::OnConnected()
{
	GProtobufSessionManager.Add(static_pointer_cast<GameProtobufSession>(shared_from_this()));
}

void GameProtobufSession::OnDisconnected()
{
	if (_accountId && _accountId != 0) {
		DebugLog::PrintColorText(LogColor::YELLOW, "[AccountId-> ", to_string(_accountId), false, false);
	}
	if (_player) {
		DebugLog::PrintColorText(LogColor::YELLOW, " / PlayerId-> ", to_string(_player->GetPlayerId()), false, false);
	}

	DebugLog::PrintColorText(LogColor::YELLOW, "]", "", false, true);

	// Room Leave
	if (_player != nullptr)
	{
		GRoom.Leave(_player);
		GameProtobufSessionRef session = _player->GetOwnerSession();
		ClientPacketHandler::HandleDisconnect(session);
	}

	// Remove Session Ref
	GProtobufSessionManager.Remove(static_pointer_cast<GameProtobufSession>(shared_from_this()));
	
	// Session Close
	if (_player != nullptr && _player->GetOwnerSession() != nullptr)
	{
		_player->SetOwnerSession(nullptr);
	}
}

void GameProtobufSession::OnRecvPacket(BYTE* buffer, int32 len)
{
	GameProtobufSessionRef session = GetProtobufSessionRef();

	PacketHeader* recvHeader = reinterpret_cast<PacketHeader*>(buffer);

	// Log
	cout << endl;
	DebugLog::PrintColorText(LogColor::GREEN, "[RECV] [PacketId-> ", to_string(recvHeader->id), true, false);
	DebugLog::PrintColorText(LogColor::GREEN, " : ", packetIdToString[recvHeader->id], false, false);
	DebugLog::PrintColorText(LogColor::GREEN, " / Size-> ", to_string(recvHeader->size), false, false);

	if (session->_accountId && session->_accountId != 0) {
		DebugLog::PrintColorText(LogColor::GREEN, " / AccountId-> ", to_string(session->_accountId), false, false);
	}
	if (session->_player) {
		DebugLog::PrintColorText(LogColor::GREEN, " / PlayerId-> ", to_string(session->_player->GetPlayerId()), false, false);
	}
	DebugLog::PrintColorText(LogColor::GREEN, "]", "", false, true);

	ClientPacketHandler::HandlePacket(buffer, len, session);
}

void GameProtobufSession::OnSend(int32 len)
{
	cout << "OnSend Len = " << len << endl;
}

void GameProtobufSession::OnSend(int32 len, vector<SendBufferRef>& sendVec)
{
	if (sendVec.size() > 0)
	{
		GameProtobufSessionRef session = GetProtobufSessionRef();
		PacketHeader* recvHeader = reinterpret_cast<PacketHeader*>(sendVec.back()->Buffer());
		
		DebugLog::PrintColorText(LogColor::BLUE, "[SEND] [PacketId-> ", to_string(recvHeader->id), true, false);
		DebugLog::PrintColorText(LogColor::BLUE, " : ", packetIdToString[recvHeader->id], false, false);
		DebugLog::PrintColorText(LogColor::BLUE, " / Size-> ", to_string(recvHeader->size), false, false);

		if (session->_accountId && session->_accountId != 0) {
			DebugLog::PrintColorText(LogColor::BLUE, " / AccountId-> ", to_string(session->_accountId), false, false);
		}
		if (session->_player) {
			DebugLog::PrintColorText(LogColor::BLUE, " / PlayerId-> ", to_string(session->_player->GetPlayerId()), false, false);
		}
		DebugLog::PrintColorText(LogColor::BLUE, "]", "", false, true);
	}
}