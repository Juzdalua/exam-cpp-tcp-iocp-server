#include "pch.h"
#include "Listener.h"
#include "SocketUtils.h"
#include "IocpEvent.h"
#include "Session.h"
#include "Service.h"

/*------------------------
	Listener
------------------------*/
Listener::~Listener()
{
	SocketUtils::Close(_listenSocket);

	for (AcceptEvent* acceptEvent : _acceptEvents)
	{
		// TODO

		delete(acceptEvent);
	}
}

bool Listener::StartAccept(ServerServiceRef service)
{
	_service = service;
	if (_service == nullptr)
		return false;

	_listenSocket = SocketUtils::CreateSocket();
	if (_listenSocket == INVALID_SOCKET)
	{
		int32 errorCode = WSAGetLastError();
		cout << errorCode << endl;
		return false;
	}
	cout << "===== Create Listen Socket =====" << endl;

	if (_service->GetIocpCore()->Register(shared_from_this()) == false)
		return false;

	if (SocketUtils::SetReuseAddress(_listenSocket, true) == false)
		return false;

	if (SocketUtils::SetLinger(_listenSocket, 0, 0) == false)
		return false;

	if (SocketUtils::Bind(_listenSocket, _service->GetNetAddress()) == false)
		return false;

	if (SocketUtils::Listen(_listenSocket) == false)
		return false;
	cout << "===== Set IOCP Done =====" << endl;

	const int32 acceptCount = _service->GetMaxSessionCount();
	for (int32 i = 0; i < acceptCount; i++) {
		AcceptEvent* acceptEvent = new AcceptEvent;
		acceptEvent->owner = shared_from_this();
		_acceptEvents.push_back(acceptEvent);
		RegisterAccept(acceptEvent);
	}

	cout << "===== AcceptEx Register Done =====" << endl;
	return true;
}

void Listener::CloseSocket()
{
	SocketUtils::Close(_listenSocket);
}

HANDLE Listener::GetHandle()
{
	return reinterpret_cast<HANDLE>(_listenSocket);
}

void Listener::RegisterAccept(AcceptEvent* acceptEvent)
{
	SessionRef session = _service->CreateSession(); // Register IOCP

	acceptEvent->Init();
	acceptEvent->session = session;
	DWORD bytesReceived = 0;

	if (false == SocketUtils::AcceptEx(_listenSocket, session->GetSocket(), session->_recvBuffer.WritePos(), 0, sizeof(SOCKADDR_IN) + 16, sizeof(SOCKADDR_IN) + 16, OUT & bytesReceived, static_cast<LPOVERLAPPED>(acceptEvent)))
	{
		const int32 errorCode = WSAGetLastError();

		if (errorCode != WSA_IO_PENDING)
		{
			// Error -> 다시 Accept 시도
			RegisterAccept(acceptEvent);
		}
	}
}

void Listener::Dispatch(IocpEvent* iocpEvent, int32 numOfBytes)
{
	ASSERT_CRASH(iocpEvent->eventType == EventType::Accept);

	AcceptEvent* acceptEvent = static_cast<AcceptEvent*>(iocpEvent);
	ProcessAccept(acceptEvent);
}

void Listener::ProcessAccept(AcceptEvent* acceptEvent)
{
	SessionRef session = acceptEvent->session;

	if (false == SocketUtils::SetUpdateAcceptSocket(session->GetSocket(), _listenSocket))
	{
		RegisterAccept(acceptEvent);
		return;
	}

	SOCKADDR_IN sockAddress;
	int32 sizeOfSockAddr = sizeof(sockAddress);
	if (SOCKET_ERROR == getpeername(session->GetSocket(), OUT reinterpret_cast<SOCKADDR*>(&sockAddress), &sizeOfSockAddr))
	{
		RegisterAccept(acceptEvent);
		return;
	}

	session->SetNetAddress(NetAddress(sockAddress));
	DebugLog::PrintColorText(LogColor::YELLOW, "[Client Connected]", "", true, true);

	session->ProcessConnect();
	RegisterAccept(acceptEvent);
}
