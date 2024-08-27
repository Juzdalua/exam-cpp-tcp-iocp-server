#include "pch.h"
#include "SocketUtils.h"
#include "IocpCore.h"
#include "Session.h"
#include "IocpEvent.h"
#include <thread>

int32 MAX_CLIENT_COUNT = 5;

// Main
int main()
{
	// Socket Set
	SocketUtils::Init();
	SOCKET listenSocket = SocketUtils::CreateSocket();
	SocketUtils::SetReuseAddress(listenSocket, true);
	SocketUtils::SetLinger(listenSocket, 0, 0);

	SocketUtils::BindAnyAddress(listenSocket, 7777);
	SocketUtils::Listen(listenSocket);
	cout << "Socket Listen Start" << endl;

	//// IOCP Set
	shared_ptr<IocpCore> iocpCoreRef = make_shared<IocpCore>();
	iocpCoreRef->Register(listenSocket);

	cout << "IOCP Set Done" << endl;

	// Client
	vector<IocpEvent*> iocpEvents;

	// AcceptEx
	iocpCoreRef->RegisterAccept(listenSocket);
	cout << "Start AcceptEx" << endl;

	// Worker Threads
	vector<thread> workers;
	for (int32 i = 0; i < 5; i++)
	{
		workers.emplace_back([&]() 
			{
				while (true) {
					iocpCoreRef->Dispatch(listenSocket);
				}
			}
		);
		//workers.push_back(thread(WorkerThreadMain, iocpCoreRef->GetHandle()));
	}
	cout << "Worker Thread Start" << endl;

	for (thread& t : workers)
	{
		if (t.joinable())
			t.join();
	}

	for ( IocpEvent* iocpEvent: iocpEvents)
	{
		delete iocpEvent;
	}

	// Socket Close
	SocketUtils::Close(listenSocket);
	SocketUtils::Clear();
}
