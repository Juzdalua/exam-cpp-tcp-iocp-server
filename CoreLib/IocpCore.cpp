#include "pch.h"
#include "IocpCore.h"
#include "IocpEvent.h"
#include "SocketUtils.h"
#include "Session.h"

IocpCore::IocpCore()
{
	_iocpHandle = CreateIoCompletionPort(INVALID_HANDLE_VALUE, 0, 0, 0);
	ASSERT_CRASH(_iocpHandle != INVALID_HANDLE_VALUE);
	
	_sessionManager = new SessionManager();
}

IocpCore::~IocpCore()
{
	CloseHandle(_iocpHandle);
	for (IocpEvent* iocpEvent : _iocpEvents)
	{
		delete iocpEvent;
	}
	delete _sessionManager;
}

//bool IocpCore::Register(IocpObjectRef iocpObject)
//{
//	return CreateIoCompletionPort(iocpObject->GetHandle(), _iocpHandle, /*key*/0, 0);
//}
//
//bool IocpCore::Dispatch(uint32 timeoutMs)
//{
//	DWORD numOfBytes = 0;
//	ULONG_PTR key = 0;
//	IocpEvent* iocpEvent = nullptr;
//
//	if (GetQueuedCompletionStatus(_iocpHandle, OUT & numOfBytes, OUT & key, OUT reinterpret_cast<LPOVERLAPPED*>(&iocpEvent), timeoutMs))
//	{
//		IocpObjectRef iocpObject = iocpEvent->owner; // Session or Listener
//		iocpObject->Dispatch(iocpEvent, numOfBytes);
//	}
//	else
//	{
//		int32 errCode = WSAGetLastError();
//
//		switch (errCode)
//		{
//		case WAIT_TIMEOUT:
//			return false;
//		default:
//			// TODO: 로그 찍기
//			IocpObjectRef iocpObject = iocpEvent->owner;
//			iocpObject->Dispatch(iocpEvent, numOfBytes);
//			break;
//		}
//	}
//
//	return true;
//}

void IocpCore::HandleError(int32 errorCode)
{
	switch (errorCode)
	{
	case WSAECONNRESET:
	case WSAECONNABORTED:
		//Disconnect(L"HandleError");
		break;

	default:
		// TODO: to log therad
		cout << "Handle Error: " << errorCode << endl;
		break;
	}
}

bool IocpCore::Register(SOCKET& socket)
{
	IsSocketValid(socket);
	return CreateIoCompletionPort((HANDLE)socket, _iocpHandle, /*key*/0, 0);
}

bool IocpCore::Register(shared_ptr<Session> session)
{
	IsSocketValid(session->GetSocket());
	return CreateIoCompletionPort((HANDLE)(session->GetSocket()), _iocpHandle, /*key*/0, 0);
}

bool IocpCore::Dispatch()
{
	DWORD numOfBytes = 0;
	ULONG_PTR key = 0;
	IocpEvent* iocpEvent = nullptr;

	if (GetQueuedCompletionStatus(_iocpHandle, OUT &numOfBytes, OUT &key, OUT reinterpret_cast<LPOVERLAPPED*>(&iocpEvent), INFINITE))
	{
		if (iocpEvent && iocpEvent->_session) {
			if (!IsSocketValid(iocpEvent->_session->GetSocket())) {
				cout << "Socket Invalid After IOCP Dispatch" << endl;
				RegisterAccept(_listenSocket);
				return false;
			}
			ProcessWorker(iocpEvent, numOfBytes);
		}
		else {
			cout << "Invalid IOCP Event or Session" << endl;
		}
	}
	else
	{
		int32 errCode = WSAGetLastError();

		if (errCode != WSA_IO_PENDING)
		{
			cout << "GetQueuedCompletionStatus Error: " << errCode << endl;
			return false;
		}
	}

	return true;
}

void IocpCore::RegisterAccept(SOCKET& listenSocket)
{
	if(_listenSocket == INVALID_SOCKET)
		_listenSocket = listenSocket;
	for (int32 i = 0; i < MAX_CLIENT_COUNT; i++)
	{
		IocpEvent* iocpEvent = new IocpEvent(EventType::Accept);
		_iocpEvents.push_back(iocpEvent);

		shared_ptr<Session> session = make_shared<Session>();
		Register(session);

		iocpEvent->Init();
		iocpEvent->_session = session;
		DWORD numOfBytes = 0;

		if (!IsSocketValid(iocpEvent->_session->GetSocket()) || !IsSocketValid(_listenSocket))
		{
			cout << "Invalid Session Socket" << endl;
			continue; // Skip this session
		}
		
		if (false == SocketUtils::AcceptEx(_listenSocket, session->GetSocket(), &(session->_buffer[0]), 0, sizeof(SOCKADDR_IN) + 16, sizeof(SOCKADDR_IN) + 16, OUT & numOfBytes, static_cast<LPOVERLAPPED>(iocpEvent)))
		{
			const int32 errorCode = WSAGetLastError();

			if (errorCode != WSA_IO_PENDING)
			{
				// Error -> 다시 Accept 시도
				cout << "Accept Error: " << errorCode << endl;
				RegisterAccept(listenSocket);
				return;
			}
		}

	}
}

void IocpCore::ProcessWorker(IocpEvent* iocpEvent, DWORD numOfBytes)
{
	switch (iocpEvent->eventType)
	{
	case EventType::Accept:
	{
		cout << "Process Accept Start" << endl;
		SessionRef session = iocpEvent->_session;

		if (false == SocketUtils::SetUpdateAcceptSocket(session->GetSocket(), _listenSocket))
		{
			int32 errCode = WSAGetLastError();
			HandleError(errCode);
			RegisterAccept(_listenSocket);
			return;
		}
		cout << "SetUpdateAcceptSocket Done" << endl;

		SOCKADDR_IN sockAddress;
		int32 sizeOfSockAddr = sizeof(sockAddress);
		if (SOCKET_ERROR == getpeername(iocpEvent->_session->GetSocket(), OUT reinterpret_cast<SOCKADDR*>(&sockAddress), &sizeOfSockAddr))
		{
			int32 errCode = WSAGetLastError();
			HandleError(errCode);
			RegisterAccept(_listenSocket);
			return;
		}
		cout << "Check Socket Addr Done" << endl;

		iocpEvent->_session->SetNetAddress(NetAddress(sockAddress));
		cout << "Client Connected" << endl;
		
		IocpEvent* connectEvent = new IocpEvent(EventType::Connect);
		connectEvent->Init();
		connectEvent->_session = iocpEvent->_session;
		ProcessWorker(connectEvent, numOfBytes);

		RegisterAccept(_listenSocket);
	}

		break;

	case EventType::Connect:
		cout << "Process Connect Start" << endl;
		iocpEvent->_session->_connected.store(true);

		// 세션 등록
		{
			mutex m;
			lock_guard<mutex> lock(m);
			_sessionManager->_sessions.insert(iocpEvent->_session);
		}

		// 컨텐츠 코드에서 오버로딩
		//OnConnected();

		// 수신 등록
		//RegisterRecv();
		break;

	case EventType::Disconnect:
		cout << "Process Disconnect Start" << endl;
		break;

	case EventType::Recv:
		cout << "Process Recv Start" << endl;
		break;

	case EventType::Send:
		cout << "Process Send Start" << endl;
		break;
	}
}
