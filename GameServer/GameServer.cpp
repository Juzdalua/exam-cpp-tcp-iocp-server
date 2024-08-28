#include "pch.h"
#include "SocketUtils.h"
#include "IocpCore.h"
#include "Session.h"
#include "IocpEvent.h"
#include <thread>

int32 MAX_CLIENT_COUNT = 5;
int32 MAX_WORKER_COUNT = 1;

// Main
int main()
{
	// Socket Set
	SocketUtils::Init();
	SOCKET listenSocket = SocketUtils::CreateSocket();
	SocketUtils::SetReuseAddress(listenSocket, true);
	SocketUtils::SetLinger(listenSocket, 0, 0);

	//SocketUtils::BindAnyAddress(listenSocket, 7777);
	SocketUtils::Bind(listenSocket, NetAddress(L"127.0.0.1", 7777));
	SocketUtils::Listen(listenSocket);
	cout << "1. Socket Listen Start" << endl;

	//// IOCP Set
	shared_ptr<IocpCore> iocpCoreRef = make_shared<IocpCore>();
	iocpCoreRef->Register(listenSocket);
	cout << "2. IOCP Set Done" << endl;

	// AcceptEx
	iocpCoreRef->RegisterAccept(listenSocket);
	cout << "3. Start AcceptEx" << endl;

	// Worker Threads
	vector<thread> workers;
	for (int32 i = 0; i < MAX_WORKER_COUNT; i++)
	{
		workers.emplace_back([&]()
			{
				while (true) {
					iocpCoreRef->Dispatch();
				}
			}
		);
	}
	cout << "4. Worker Thread Start" << endl;

	for (thread& t : workers)
	{
		if (t.joinable())
			t.join();
	}
	cout << "5. Worker Thread Exit" << endl;

	// Socket Close
	SocketUtils::Close(listenSocket);
	SocketUtils::Clear();
	cout << "6. Server has been shut down." << endl;
}
