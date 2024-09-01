#include "pch.h"
#include "Service.h"
#include "SocketUtils.h"
#include "ServerSession.h"

int32 MAX_WORKER_COUNT = 2;

// ServerSession
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

// ServerPacketSession
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

// ServerProtobufSession

int main()
{
	this_thread::sleep_for(1s);
	SocketUtils::Init();

	ClientServiceRef service = ClientServiceRef(
		new ClientService(
			NetAddress(L"127.0.0.1", 7777),
			IocpCoreRef(new IocpCore()),
			//[&]() {return shared_ptr<ServerSession>(new ServerSession());},
			//[&]() {return shared_ptr<ServerPacketSession>(new ServerPacketSession());},
			[&]() {return shared_ptr<ServerProtobufSession>(new ServerProtobufSession());},
			1
		)
	);

	ASSERT_CRASH(service->Start());

	mutex m;
	vector<thread> workers;
	for (int32 i = 0; i < MAX_WORKER_COUNT; i++)
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
	//ChatPacket(service);

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