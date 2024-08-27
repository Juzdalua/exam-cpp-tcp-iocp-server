#include "pch.h"
#include "SocketUtils.h"
#include "IocpCore.h"
#include "Session.h"
#include "IocpEvent.h"
#include <thread>

int32 MAX_CLIENT_COUNT = 1;

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
	IocpCore* iocpCore = new IocpCore();
	unique_ptr<IocpCore> uIocpCore(iocpCore);
	uIocpCore->Register(listenSocket);
	cout << "IOCP Set Done" << endl;

	// Client
	vector<IocpEvent*> iocpEvents;

	cout << "Start AcceptEx" << endl;
	// AcceptEx
	for (int32 i = 0; i < MAX_CLIENT_COUNT; i++)
	{
		shared_ptr<Session> session = make_shared<Session>();
		uIocpCore->Register(session);

		IocpEvent* iocpEvent = new IocpEvent(EventType::Accept);
		iocpEvent->Init();
		iocpEvent->_session = session;
		iocpEvents.push_back(iocpEvent);
		DWORD numOfBytes = 0;

		SocketUtils::AcceptEx(listenSocket, session->GetSocket(), &session->buffer, 0, sizeof(SOCKADDR_IN) + 16, sizeof(SOCKADDR_IN) + 16, OUT & numOfBytes, static_cast<LPOVERLAPPED>(iocpEvent));
	}

	// Worker Threads
	vector<thread> workers;
	for (int32 i = 0; i < 5; i++)
	{
		workers.emplace_back([&]() 
			{
				while (true) {
					uIocpCore->Dispatch(listenSocket);
				}
			}
		);
		//workers.push_back(thread(WorkerThreadMain, uIocpCore->GetHandle()));
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
	delete iocpCore;

	// Socket Close
	SocketUtils::Close(listenSocket);
	SocketUtils::Clear();
}
