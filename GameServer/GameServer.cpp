#include "pch.h"
#include "ConnectionPool.h"
#include "SocketUtils.h"
#include "IocpCore.h"
#include "GameSession.h"
#include "Service.h"
#include "Protocol.pb.h"
#include "Room.h"

CoreGlobal GCoreGlobal;
int32 MAX_CLIENT_COUNT = 1;
int32 MAX_WORKER_COUNT = thread::hardware_concurrency();


// System Server Message
void SystemMessageFromServer()
{
	int32 i = 0;
	while (true)
	{
		string input;
		getline(cin, input);

		uint16 packetId = PKT_S_SERVER_CHAT;
		Protocol::S_SERVER_CHAT pkt;
		pkt.set_type(Protocol::CHAT_TYPE_SYSTEM);
		pkt.set_msg(input);

		cout << pkt.msg() << endl;

		SendBufferRef sendBuffer = MakeSendBuffer(pkt, packetId);

		GRoom.Broadcast(sendBuffer);
		break;
	}
}

int main()
{
	SocketUtils::Init();
	cout << "Max Thread Count: " << MAX_WORKER_COUNT << endl;

	ServerServiceRef service = ServerServiceRef(
		new ServerService(
			NetAddress(L"127.0.0.1", 7777),
			IocpCoreRef(new IocpCore()),
			//[&]() {return shared_ptr<GameSession>(new GameSession());},
			//[&]() {return shared_ptr<GamePacketSession>(new GamePacketSession());},
			[&]() {return shared_ptr<GameProtobufSession>(new GameProtobufSession());},
			MAX_CLIENT_COUNT
		)
	);

	ASSERT_CRASH(service->Start());

	// Worker Threads
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
	cout << "===== Worker Thread Start =====" << endl;

	//SystemMessageFromServer();

	for (thread& t : workers)
	{
		if (t.joinable())
			t.join();
	}
	cout << "===== Worker Thread Exit =====" << endl;
	SocketUtils::Clear();
	cout << "===== Server has been shut down. =====" << endl;
}

/*
	Listener -> Socket Set -> Register Accept (AcceptEx)
	Process Accept -> Client Session Set -> Process Connect -> Register Recv (WSARecv)
	Process Recv -> GameSession Echo Set -> Send with echo -> Register Recv (WSARecv)
	SendBuffer Set -> Send (SendQueue Set) -> RegisterSend (WSASend) -> ProcessSend -> Register Send (WSASend)
*/