#include "pch.h"
#include "IocpCore.h"
#include "GameSession.h"
#include "Service.h"
#include "Protocol.pb.h"
#include "PacketPriorityQueue.h"

CoreGlobal GCoreGlobal;
int32 MAX_CLIENT_COUNT = 1;
int32 MAX_WORKER_COUNT = thread::hardware_concurrency();

void DoWorkerJob(ServerServiceRef& service)
{
	while (true) {
		if (GPacketPriorityQueue->IsEmpty())
		{
			service->GetIocpCore()->Dispatch(10);
		}
		GPacketPriorityQueue->ProcessPackets();
	}
}

int main()
{
	GPacketPriorityQueue = new PacketPriorityQueue();

	cout << "Max Thread Count: " << MAX_WORKER_COUNT << endl;

	ServerServiceRef service = ServerServiceRef(
		new ServerService(
			NetAddress(L"127.0.0.1", 7777),
			IocpCoreRef(new IocpCore()),
			[&]() {return GameProtobufSessionRef(new GameProtobufSession());},
			MAX_CLIENT_COUNT
		)
	);

	ASSERT_CRASH(service->Start());

	// Worker Threads
	vector<thread> workers;
	for (int32 i = 0; i < MAX_WORKER_COUNT; i++)
	{
		workers.push_back(thread([&]()
			{
				DoWorkerJob(service);
			}));
	}
	cout << "===== Worker Thread Start =====" << endl;

	for (thread& t : workers)
	{
		if (t.joinable())
			t.join();
	}
	cout << "===== Worker Thread Exit =====" << endl;
	cout << "===== Server has been shut down. =====" << endl;

	if (GPacketPriorityQueue != nullptr) {
		delete GPacketPriorityQueue;
		GPacketPriorityQueue = nullptr;
	}
}

/*
	Listener -> Socket Set -> Register Accept (AcceptEx)
	Process Accept -> Client Session Set -> Process Connect -> Register Recv (WSARecv)
	Process Recv -> GameSession Echo Set -> Send with echo -> Register Recv (WSARecv)
	SendBuffer Set -> Send (SendQueue Set) -> RegisterSend (WSASend) -> ProcessSend -> Register Send (WSASend)
*/