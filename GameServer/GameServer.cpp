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
	SessionManager* sessions = new SessionManager();
	vector<IocpEvent*> iocpEvents;

	cout << "Start AcceptEx" << endl;
	// AcceptEx
	for (int32 i = 0; i < MAX_CLIENT_COUNT; i++)
	{
		Session* session = new Session();
		sessions->_sessionManager.push_back(session);
		SOCKET clientSocket = session->GetSocket();
		uIocpCore->Register(clientSocket);

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

	//while (true)
	//{
	//	// 7. Accept Client Socket
	//	SOCKADDR_IN clientAddr;
	//	int addrLen = sizeof(clientAddr);
	//	SOCKET clientSocket = accept(listenSocket, (SOCKADDR*)&clientAddr, &addrLen);
	//	if (clientSocket == INVALID_SOCKET) {
	//		HandleError("Accept");
	//		return 0;
	//	}
	//	
	//	shared_ptr<Session> sessionRef = make_shared<Session>();

	//	cout << "Client Connected" << endl;

	//	CreateIoCompletionPort((HANDLE)clientSocket, uIocpCore->GetHandle(), 0, 0);

	//	// 8. Register Client Socket to IOCP
	//	WSABUF wsaBuf;
	//	wsaBuf.buf = sessionRef->GetRecvBuffer();
	//	wsaBuf.len = BUFSIZE;

	//	DWORD recvLen = 0;
	//	DWORD flags = 0;

	//	IocpEvent* iocpEvent = new IocpEvent(EventType::Recv);

	//	// 9. WSARecv
	//	WSARecv(clientSocket, &wsaBuf, 1, &recvLen, &flags, iocpEvent, NULL);
	//}


	//for (auto& t : workers)
	//{
	//	if (t.joinable())
	//		t.join();
	//}



	for ( IocpEvent* iocpEvent: iocpEvents)
	{
		delete iocpEvent;
	}
	delete iocpCore;
	delete sessions;

	// Socket Close
	SocketUtils::Close(listenSocket);
	SocketUtils::Clear();
}
