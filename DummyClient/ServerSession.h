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

//class ServerProtobufSession : public PacketSession
//{
//public:
//	ServerProtobufSession() {};
//	virtual ~ServerProtobufSession() {};
//
//	virtual void OnConnected() override
//	{
//		cout << "===== Connected To Server =====" << endl;
//
//		Protocol::C_CHAT pkt;
//		pkt.set_msg("Ping");
//		uint16 packetId = PKT_C_TEST;
//
//		Send(MakeSendBuffer(pkt, packetId));
//	}
//
//	virtual void OnRecvPacket(BYTE* buffer, int32 len) override
//	{
//		PacketSessionRef session = GetPacketSessionRef();
//		PacketHeader* header = reinterpret_cast<PacketHeader*>(buffer);
//		
//		switch (header->id)
//		{
//		case PKT_S_TEST:
//			Protocol::S_CHAT recvPkt;
//			recvPkt.ParseFromArray(buffer + sizeof(PacketHeader), len - sizeof(PacketHeader));
//			
//			thread([=]() {
//				this_thread::sleep_for(3s);
//
//				Protocol::C_CHAT pkt;
//				pkt.set_msg("Ping");
//				uint16 packetId = PKT_C_TEST;
//
//				Send(MakeSendBuffer(pkt, packetId));
//				}).detach();
//
//			break;
//		}
//	}
//
//	virtual void OnSend(int32 len) override
//	{
//		cout << "OnSend Len = " << len << endl;
//	}
//
//	virtual void OnDisconnected() override
//	{
//		cout << "Disconnected" << endl;
//	}
//};


class ServerProtobufSession : public PacketSession
{
public:
	ServerProtobufSession() {};
	virtual ~ServerProtobufSession() {};

	virtual void OnConnected() override
	{
		cout << "===== Connected To Server =====" << endl;

		// Set initial next send time to 3 seconds from now
		nextSendTime = std::chrono::steady_clock::now() + std::chrono::seconds(3);

		// Start a thread to manage periodic ping sending
		timerThread = std::thread([this]() {
			while (true)
			{
				auto now = std::chrono::steady_clock::now();

				// If it's time to send the packet
				if (now >= nextSendTime)
				{
					// Send the ping packet
					SendPing();

					// Update the next send time to be 3 seconds later
					nextSendTime = now + std::chrono::seconds(3);
				}

				// Small sleep to avoid busy waiting
				std::this_thread::sleep_for(std::chrono::milliseconds(10));
			}
			});
	}

	virtual void OnRecvPacket(BYTE* buffer, int32 len) override
	{
		PacketSessionRef session = GetPacketSessionRef();
		PacketHeader* header = reinterpret_cast<PacketHeader*>(buffer);

		switch (header->id)
		{
		case PKT_S_TEST:
			Protocol::S_CHAT recvPkt;
			recvPkt.ParseFromArray(buffer + sizeof(PacketHeader), len - sizeof(PacketHeader));
			// Optionally handle the received packet
			break;
		}
	}

	virtual void OnSend(int32 len) override
	{
		cout << "OnSend Len = " << len << endl;
	}

	virtual void OnDisconnected() override
	{
		stopRequested = true;
		if (timerThread.joinable())
		{
			timerThread.join();
		}
		cout << "Disconnected" << endl;
	}

private:
	void SendPing()
	{
		Protocol::C_CHAT pkt;
		pkt.set_msg("Ping");
		uint16 packetId = PKT_C_TEST;

		// Ensure thread-safe output
		{
			lock_guard<mutex> guard(cout_mutex);
			cout << "Send Client Ping" << endl;
		}

		Send(MakeSendBuffer(pkt, packetId));
	}

	std::chrono::steady_clock::time_point nextSendTime;
	std::thread timerThread;
	std::atomic<bool> stopRequested{ false };
	std::mutex cout_mutex;
};
