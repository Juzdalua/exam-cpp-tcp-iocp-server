#include "pch.h"
#include "CorePch.h"

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
	// 1. WSAStartup
	WSAData wsaData;
	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
		HandleError("WSAStartup");
		return 0;
	}

	// 2. TCP Socket Set
	SOCKET listenSocket = socket(AF_INET, SOCK_STREAM,IPPROTO_TCP);
	if (listenSocket == INVALID_SOCKET)
	{
		HandleError("listenSocket");
		return 0;
	}

	// 3. Server Set
	SOCKADDR_IN serverAddr;
	memset(&serverAddr, 0, sizeof(serverAddr));
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_addr.s_addr = htonl(INADDR_ANY);
	serverAddr.sin_port = htons(7777);

	// 4. Bind
	if (::bind(listenSocket, (SOCKADDR*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR)
	{
		HandleError("Bind");
		return 0;
	}

	// 5. Listen
	if (listen(listenSocket, SOMAXCONN) == SOCKET_ERROR)
	{
		HandleError("Listen");
		return 0;
	}

	cout << "Accept" << endl;

	vector<Session*> sessionManager;


	// 6. IOCP Handle Set
	HANDLE iocpHandle = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, 0);

	thread t1(WorkerThreadMain, iocpHandle);

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
		CreateIoCompletionPort((HANDLE)clientSocket, iocpHandle, /*key*/(ULONG_PTR)session, 0);

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

	WSACleanup();
}
