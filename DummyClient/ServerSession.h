#pragma once
#include "pch.h"
#include "Session.h"
#include "Protocol.pb.h"

/*-------------------
	Server Session
	Without Header
-------------------*/
class ServerSession : public Session
{
public:
	ServerSession()
	{
	}
	virtual ~ServerSession()
	{
	}

	virtual void OnConnected() override
	{
		cout << "===== Connected To Server =====" << endl;

		SendBufferRef sendBuffer = SendBufferRef(new SendBuffer(4096));
		char initSendData[] = "Ping";
		sendBuffer->CopyData(initSendData, sizeof(initSendData));
		Send(sendBuffer);
	}

	virtual int32 OnRecv(BYTE* buffer, int32 len) override
	{
		cout << "OnRecv Len = " << len << ", OnRecv Data = " << buffer << endl;

		return len;
	}

	virtual void OnSend(int32 len) override
	{
		cout << "OnSend Len = " << len << endl;
	}

	virtual void OnDisconnected() override
	{
		cout << "Disconnected" << endl;
	}
};

/*-----------------------
	Server Packet Session
	With PacketHeader
-----------------------*/
class ServerPacketSession : public PacketSession
{
public:
	ServerPacketSession()
	{
	}
	virtual ~ServerPacketSession()
	{
	}

	virtual void OnConnected() override
	{
		cout << "===== Connected To Server =====" << endl;

		SendBufferRef sendBuffer = SendBufferRef(new SendBuffer(4096));
		char initSendData[] = "Ping";

		PacketHeader* header = reinterpret_cast<PacketHeader*>(sendBuffer->Buffer());
		header->id = 1;
		header->size = sizeof(initSendData) + sizeof(PacketHeader);

		sendBuffer->CopyPacket(&sendBuffer->Buffer()[4], &initSendData, sizeof(initSendData));

		Send(sendBuffer);
	}

	virtual void OnRecvPacket(BYTE* buffer, int32 len) override
	{
		PacketSessionRef session = GetPacketSessionRef();
		PacketHeader* header = reinterpret_cast<PacketHeader*>(buffer);

		char recvBuffer[4096];
		memcpy(recvBuffer, &buffer[4], header->size - sizeof(PacketHeader));
		cout << "Packet Id: " << header->id << ", SIze: " << header->size << endl;
		cout << recvBuffer << endl;
	}

	virtual void OnSend(int32 len) override
	{
		cout << "OnSend Len = " << len << endl;
	}

	virtual void OnDisconnected() override
	{
		cout << "Disconnected" << endl;
	}
};

/*-------------------
	Server Protobuf Session
	With PakcetHeader
-------------------*/
enum : uint16
{
	PKT_C_TEST = 1000,
	PKT_S_TEST = 1001,

	PKT_C_SIGNUP = 1002,
	PKT_S_SIGNUP = 1003,

	PKT_C_LOGIN = 1004,
	PKT_S_LOGIN = 1005,

	PKT_C_ENTER_GAME = 1006,
	PKT_S_ENTER_GAME = 1007,

	PKT_C_CHAT = 1008,
	PKT_S_CHAT = 1009,
};

class ServerProtobufSession : public PacketSession
{
public:
	ServerProtobufSession() {};
	virtual ~ServerProtobufSession() {};

	virtual void OnConnected() override
	{
		cout << "===== Connected To Server =====" << endl;

		Protocol::C_CHAT pkt;
		pkt.set_msg("Ping");
		uint16 packetId = PKT_C_TEST;

		const uint16 dataSize = static_cast<uint16>(pkt.ByteSizeLong());
		const uint16 packetSize = dataSize + sizeof(PacketHeader);

		SendBufferRef sendBuffer = SendBufferRef(new SendBuffer(4096));
		PacketHeader* header = reinterpret_cast<PacketHeader*>(sendBuffer->Buffer());
		header->size = packetSize;
		header->id = packetId;

		ASSERT_CRASH(pkt.SerializeToArray(&header[1], dataSize));
		sendBuffer->SetWriteSizeWithDataSize(dataSize);
		
		Send(sendBuffer);
	}

	virtual void OnRecvPacket(BYTE* buffer, int32 len) override
	{
		/*PacketSessionRef session = GetPacketSessionRef();
		PacketHeader* header = reinterpret_cast<PacketHeader*>(buffer);

		char recvBuffer[4096];
		memcpy(recvBuffer, &buffer[4], header->size - sizeof(PacketHeader));
		cout << "Packet Id: " << header->id << ", SIze: " << header->size << endl;
		cout << recvBuffer << endl;*/

		PacketSessionRef session = GetPacketSessionRef();
		PacketHeader* header = reinterpret_cast<PacketHeader*>(buffer);
		
		switch (header->id)
		{
		case PKT_S_TEST:
			Protocol::S_CHAT pkt;
			pkt.ParseFromArray(buffer + sizeof(PacketHeader), len - sizeof(PacketHeader));
			cout << pkt.msg() << endl;
			break;
		}
	}

	virtual void OnSend(int32 len) override
	{
		cout << "OnSend Len = " << len << endl;
	}

	virtual void OnDisconnected() override
	{
		cout << "Disconnected" << endl;
	}
};