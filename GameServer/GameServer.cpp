#include "pch.h"
#include "SocketUtils.h"
#include "IocpCore.h"
#include "Session.h"
#include "GameSession.h"
#include "Service.h"
#include <functional>

int32 MAX_CLIENT_COUNT = 1;
int32 MAX_WORKER_COUNT = 1;

int main()
{
	SocketUtils::Init();

	ServerServiceRef service = ServerServiceRef(
		new ServerService(
			NetAddress(L"127.0.0.1", 7777),
			IocpCoreRef(new IocpCore()),
			[&]() {return shared_ptr<GameSession>(new GameSession());},
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
	Send -> SendQueue & SendBuffer Set -> RegisterSend (WSASend) -> ProcessSend -> Register Send (WSASend)
*/