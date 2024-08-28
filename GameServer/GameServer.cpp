#include "pch.h"
#include "SocketUtils.h"
#include "IocpCore.h"
#include "Session.h"
#include "IocpEvent.h"
#include <thread>

int32 MAX_CLIENT_COUNT = 5;

// 서버 실행 상태를 추적하는 변수
atomic<bool> running(true); 

void WorkerThreadMain(shared_ptr<IocpCore> iocpCoreRef) 
{
	while (running) {
		iocpCoreRef->Dispatch();
	}
}

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
	for (int32 i = 0; i < 1; i++) 
	{
		workers.emplace_back(WorkerThreadMain, iocpCoreRef);
	}
	cout << "4. Worker Thread Start" << endl;

	this_thread::sleep_for(10s);

	running.store(false);
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
