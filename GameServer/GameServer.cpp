#include "pch.h"
#include "IocpCore.h"
#include "SocketUtils.h"

// Error
void HandleError(const char* cause)
{
	cout << "Error: " << cause << endl;
}

// Session
const int BUFSIZE = 1'000;
struct Session
{
	SOCKET socket = INVALID_SOCKET;
	char recvBuffer[BUFSIZE] = {};
	int recvBytes = 0;
};

// Overlapped
enum IO_TYPE
{
	READ,
	WRITE,
	ACCEPT,
	CONNECT,
};

struct OverlappedEx
{
	WSAOVERLAPPED overlapped = {};
	int type = 0; // IO_TYPE
};

// Worker Thread
void WorkerThreadMain(HANDLE iocpHandle)
{
	while (true)
	{
		DWORD bytesTransfered = 0;
		Session* session = nullptr;
		OverlappedEx* overlappedEx = nullptr;

		// Find Job
		BOOL ret = GetQueuedCompletionStatus(iocpHandle, &bytesTransfered, (ULONG_PTR*)&session, (LPOVERLAPPED*)&overlappedEx, INFINITE);
		if (ret == FALSE || bytesTransfered == 0)
		{
			continue;
		}
		cout << "Recv Data Len: " << bytesTransfered << endl;

		WSABUF wsaBuf;
		wsaBuf.buf = session->recvBuffer;
		wsaBuf.len = BUFSIZE;

		cout << "Recv Data: " << wsaBuf.buf << endl;

		// Echo
		DWORD numsOfBytes = 0;
		if (WSASend(session->socket, &wsaBuf, 1, &numsOfBytes, 0, nullptr, nullptr) == SOCKET_ERROR)
		{
			int32 errorCode = WSAGetLastError();
			if (errorCode != WSA_IO_PENDING)
			{
				HandleError("Echo Send Error");
				continue;
			}
		}

		// Wait Recv
		DWORD recvLen = 0;
		DWORD flags = 0;
		WSARecv(session->socket, &wsaBuf, 1, OUT &recvLen, OUT &flags, &overlappedEx->overlapped, NULL);
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

	vector<Session*> sessionManager;
	shared_ptr<Session> sessionRef = make_shared<Session>();

	// IOCP Set
	IocpCore* iocpCore = new IocpCore();
	unique_ptr<IocpCore> uIocpCore(iocpCore);
	CreateIoCompletionPort((HANDLE)listenSocket, uIocpCore->GetHandle(), 0, 0);

	thread t1(WorkerThreadMain, uIocpCore->GetHandle());

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

		Session* session = new Session();
		session->socket = clientSocket;
		sessionManager.push_back(session);

		cout << "Client Connected" << endl;

		// 8. Register Client Socket to IOCP
		//CreateIoCompletionPort((HANDLE)clientSocket, iocpHandle, /*key*/(ULONG_PTR)session, 0);

		WSABUF wsaBuf;
		wsaBuf.buf = session->recvBuffer;
		wsaBuf.len = BUFSIZE;

		DWORD recvLen = 0;
		DWORD flags = 0;

		OverlappedEx* overlappedEx = new OverlappedEx();
		overlappedEx->type = IO_TYPE::READ;

		// 9. WSARecv
		WSARecv(clientSocket, &wsaBuf, 1, &recvLen, &flags, &overlappedEx->overlapped, NULL);
	}

	if (t1.joinable())
		t1.join();

	// Socket Close
	SocketUtils::Close(listenSocket);
	SocketUtils::Clear();
}
