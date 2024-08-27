#include "pch.h"
#include "IocpCore.h"
#include "SocketUtils.h"
#include "IocpEvent.h"

// Error
void HandleError(const char* cause)
{
	cout << "Error: " << cause << endl;
}

// Session
const int BUFSIZE = 1'000;

// Worker Thread
void WorkerThreadMain(HANDLE iocpHandle)
{
	while (true)
	{
		ULONG_PTR key = 0;
		DWORD bytesTransfered = 0;
		//Session* session = nullptr;
		IocpEvent* iocpEvent = nullptr;

		// Find Job
		BOOL ret = GetQueuedCompletionStatus(iocpHandle, &bytesTransfered, &key, (LPOVERLAPPED*)&iocpEvent, INFINITE);
		if (ret == FALSE || bytesTransfered == 0)
		{
			continue;
		}
		cout << "Recv Data Len: " << bytesTransfered << endl;

		WSABUF wsaBuf;
		//wsaBuf.buf = session->recvBuffer;
		wsaBuf.len = BUFSIZE;

		//cout << "Recv Data: " << wsaBuf.buf << endl;

		// Echo
		/*DWORD numsOfBytes = 0;
		if (WSASend(session->socket, &wsaBuf, 1, &numsOfBytes, 0, nullptr, nullptr) == SOCKET_ERROR)
		{
			int32 errorCode = WSAGetLastError();
			if (errorCode != WSA_IO_PENDING)
			{
				HandleError("Echo Send Error");
				continue;
			}
		}*/

		// Wait Recv
		DWORD recvLen = 0;
		DWORD flags = 0;
		//WSARecv(session->socket, &wsaBuf, 1, OUT &recvLen, OUT &flags, &overlappedEx->overlapped, NULL);
	}
}

// Main
int main()
{
	// Socket Set
	SocketUtils::Init();
	SOCKET listenSocket = SocketUtils::CreateSocket();
	SocketUtils::BindAnyAddress(listenSocket, 7777);
	SocketUtils::Listen(listenSocket);
	cout << "Accept" << endl;

	// IOCP Set
	IocpCore* iocpCore = new IocpCore();
	unique_ptr<IocpCore> uIocpCore(iocpCore);

	// Worker Threads
	vector<thread> workers;
	for (int32 i = 0; i < 5; i++)
	{
		workers.emplace_back([&]() {WorkerThreadMain(uIocpCore->GetHandle());});
		//workers.push_back(thread(WorkerThreadMain, uIocpCore->GetHandle()));
	}

	while (true)
	{
		// 7. Accept Client Socket
		SOCKADDR_IN clientAddr;
		int addrLen = sizeof(clientAddr);
		SOCKET clientSocket = accept(listenSocket, (SOCKADDR*)&clientAddr, &addrLen);
		if (clientSocket == INVALID_SOCKET) {
			HandleError("Accept");
			return 0;
		}
		
		shared_ptr<Session> sessionRef = make_shared<Session>();

		cout << "Client Connected" << endl;

		CreateIoCompletionPort((HANDLE)clientSocket, uIocpCore->GetHandle(), 0, 0);

		// 8. Register Client Socket to IOCP
		WSABUF wsaBuf;
		wsaBuf.buf = sessionRef->GetRecvBuffer();
		wsaBuf.len = BUFSIZE;

		DWORD recvLen = 0;
		DWORD flags = 0;

		IocpEvent* iocpEvent = new IocpEvent(EventType::Recv);

		// 9. WSARecv
		WSARecv(clientSocket, &wsaBuf, 1, &recvLen, &flags, iocpEvent, NULL);
	}





	for (auto& t : workers)
	{
		if (t.joinable())
			t.join();
	}

	// Socket Close
	SocketUtils::Close(listenSocket);
	SocketUtils::Clear();
}
