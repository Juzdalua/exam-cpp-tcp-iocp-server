#include "pch.h"
#include "SocketUtils.h"
#include "Session.h"

LPFN_CONNECTEX		SocketUtils::ConnectEx = nullptr;
LPFN_DISCONNECTEX	SocketUtils::DisconnectEx = nullptr;
LPFN_ACCEPTEX		SocketUtils::AcceptEx = nullptr;
vector<IocpEvent*> SocketUtils::_iocpEvents = {};

void SocketUtils::Init()
{
	WSADATA wsaData;
	ASSERT_CRASH(WSAStartup(MAKEWORD(2, 2), OUT &wsaData) == 0);

	/* 런타임에 주소 얻어오는 API */
	SOCKET dummySocket = CreateSocket();
	BindWindowsFunction(dummySocket, WSAID_CONNECTEX, reinterpret_cast<LPVOID*>(&ConnectEx));
	BindWindowsFunction(dummySocket, WSAID_DISCONNECTEX, reinterpret_cast<LPVOID*>(&DisconnectEx));
	BindWindowsFunction(dummySocket, WSAID_ACCEPTEX, reinterpret_cast<LPVOID*>(&AcceptEx));
	Close(dummySocket);
}

void SocketUtils::Clear()
{
	WSACleanup();
}

bool SocketUtils::BindWindowsFunction(SOCKET socket, GUID guid, LPVOID* fn)
{
	DWORD bytes = 0;
	return SOCKET_ERROR != WSAIoctl(socket, SIO_GET_EXTENSION_FUNCTION_POINTER, &guid, sizeof(guid), fn, sizeof(*fn), OUT &bytes, NULL, NULL);
}

SOCKET SocketUtils::CreateSocket()
{
	return WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, NULL, 0, WSA_FLAG_OVERLAPPED);
}

bool SocketUtils::Bind(SOCKET socket, wstring ip, uint16 port)
{
	return false;
}

bool SocketUtils::BindAnyAddress(SOCKET listenSocket, uint16 port)
{
	SOCKADDR_IN myAddress;
	myAddress.sin_family = AF_INET;
	myAddress.sin_addr.s_addr = htonl(INADDR_ANY);
	myAddress.sin_port = htons(port);

	return SOCKET_ERROR != ::bind(listenSocket, reinterpret_cast<const SOCKADDR*>(&myAddress), sizeof(myAddress));
}

bool SocketUtils::Listen(SOCKET listenSocket, int32 backlog)
{
	return SOCKET_ERROR != listen(listenSocket, backlog);
}

void SocketUtils::Accept(SOCKET listenSocket)
{
	for (int32 i = 0; i < 5; i++)
	{
		shared_ptr<Session> sessionRef = make_shared<Session>();
		GSessionManager._sessionManager.push_back(sessionRef);
		
		DWORD bytesReceived = 0;
		IocpEvent* iocpEvent = new IocpEvent(EventType::Accept);
		iocpEvent->Init();
		_iocpEvents.push_back(iocpEvent);

		if (false == AcceptEx(listenSocket, sessionRef->GetSocket(), sessionRef->_recvBuffer, 0, sizeof(SOCKADDR_IN) + 16, sizeof(SOCKADDR_IN) + 16, OUT &bytesReceived, static_cast<LPOVERLAPPED>(iocpEvent)))
		{
			const int32 errorCode = WSAGetLastError();

			if (errorCode != WSA_IO_PENDING)
			{
				// Error -> 다시 Accept 시도
				SocketUtils::Accept(listenSocket);
			}
		}
	}
}


void SocketUtils::Close(SOCKET& socket)
{
	if (socket != INVALID_SOCKET)
		closesocket(socket);
	socket = INVALID_SOCKET;
}
