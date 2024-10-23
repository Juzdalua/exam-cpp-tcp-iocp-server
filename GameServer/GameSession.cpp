#pragma once
#include "pch.h"
#include "GameSession.h"
#include "GameSessionManager.h"
#include "Protocol.pb.h"
#include "Room.h"
#include "Player.h"
#include "ClientPacketHandler.h"
#include "PacketUtils.h"

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
	GameProtobufSessionRef session = _player->GetOwnerSession();
	logQueue.Push({ LogType::DISCONNECT, LogColor::YELLOW , nullptr, session });

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

	packetQueue.Push({ buffer, len, session });
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

		logQueue.Push({ LogType::SEND, LogColor::BLUE , recvHeader, session });
	}
}

void HandlePacketStartLog(string type, LogColor color, PacketHeader* recvHeader, GameProtobufSessionRef& session)
{
	DebugLog::PrintColorText(color, "[" + type + "]", "", true, false);
	if (recvHeader != nullptr)
	{
		DebugLog::PrintColorText(color, "PacketId-> ", to_string(recvHeader->id), false, false);
		DebugLog::PrintColorText(color, " : ", packetIdToString[recvHeader->id], false, false);
		DebugLog::PrintColorText(color, " / Size-> ", to_string(recvHeader->size), false, false);
	}
	if (session != nullptr)
	{
		if (session->_accountId && session->_accountId != 0) {
			DebugLog::PrintColorText(color, " / AccountId-> ", to_string(session->_accountId), false, false);
		}
		if (session->_player) {
			DebugLog::PrintColorText(color, " / PlayerId-> ", to_string(session->_player->GetPlayerId()), false, false);
		}
	}
	DebugLog::PrintColorText(color, "", "", false, true);
}
