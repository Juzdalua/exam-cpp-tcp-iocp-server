#include "pch.h"
#include "Service.h"
#include "IocpCore.h"
#include "SocketUtils.h"
#include "Protocol.pb.h"

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

void Chat(ClientServiceRef service)
{
	while (true)
	{
		char sendData[100];
		cin >> sendData;

		SendBufferRef sendBuffer = SendBufferRef(new SendBuffer(4096));
		sendBuffer->CopyData(sendData, sizeof(sendData));
		service->Broadcast(sendBuffer);
	}
}

void ChatPacket(ClientServiceRef service)
{
	while (true)
	{
		char sendData[100];
		cin >> sendData;

		SendBufferRef sendBuffer = SendBufferRef(new SendBuffer(4096));
		PacketHeader* header = reinterpret_cast<PacketHeader*>(sendBuffer->Buffer());
		header->id = 1;
		header->size = sizeof(sendData) + sizeof(PacketHeader);
		sendBuffer->CopyPacket(&sendBuffer->Buffer()[4], &sendData, sizeof(sendData));
		service->Broadcast(sendBuffer);
	}
}

int main()
{
	SocketUtils::Init();

	ClientServiceRef service = ClientServiceRef(
		new ClientService(
			NetAddress(L"127.0.0.1", 7777),
			IocpCoreRef(new IocpCore()),
			//[&]() {return shared_ptr<ServerSession>(new ServerSession());},
			[&]() {return shared_ptr<ServerPacketSession>(new ServerPacketSession());},
			1
		)
	);

	ASSERT_CRASH(service->Start());

	mutex m;
	vector<thread> workers;
	for (int32 i = 0; i < 2; i++)
	{
		workers.push_back(thread([&]()
			{
				while (true) {
					lock_guard<mutex> guard(m);
					service->GetIocpCore()->Dispatch();
				}
			}));
	}

	// Chat
	//Chat(service);
	ChatPacket(service);

	// Room
	/*Protocol::C_CHAT chatPkt;
	chatPkt.set_msg(u8"Hello World");

	SendBufferRef sendBuffer = SendBufferRef(new SendBuffer(4096));
	sendBuffer->CopyData(&chatPkt, chatPkt.ByteSizeLong());
	service->Broadcast(sendBuffer);

	while (true)
	{
		service->Broadcast(sendBuffer);
		this_thread::sleep_for(1s);
	}*/
	
	for (thread& t : workers)
	{
		if (t.joinable())
			t.join();
	}

	SocketUtils::Clear();
}