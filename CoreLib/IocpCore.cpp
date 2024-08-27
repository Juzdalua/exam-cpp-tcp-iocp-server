#include "pch.h"
#include "IocpCore.h"
#include "IocpEvent.h"
#include "SocketUtils.h"
#include "Session.h"

IocpCore::IocpCore()
{
	_iocpHandle = CreateIoCompletionPort(INVALID_HANDLE_VALUE, 0, 0, 0);
	ASSERT_CRASH(_iocpHandle != INVALID_HANDLE_VALUE);
}

IocpCore::~IocpCore()
{
	CloseHandle(_iocpHandle);
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
//			// TODO: ·Î±× Âï±â
//			IocpObjectRef iocpObject = iocpEvent->owner;
//			iocpObject->Dispatch(iocpEvent, numOfBytes);
//			break;
//		}
//	}
//
//	return true;
//}

bool IocpCore::Register(SOCKET& socket)
{
	return CreateIoCompletionPort((HANDLE)socket, _iocpHandle, /*key*/0, 0);
}

bool IocpCore::Dispatch(SOCKET& listenSocket)
{
	DWORD numOfBytes = 0;
	ULONG_PTR key = 0;
	IocpEvent* iocpEvent = nullptr;

	if (GetQueuedCompletionStatus(_iocpHandle, OUT &numOfBytes, OUT &key, OUT reinterpret_cast<LPOVERLAPPED*>(&iocpEvent), INFINITE))
	{
		// TODO - toss to worker
		ProcessWorker(iocpEvent, numOfBytes, listenSocket);
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

void IocpCore::ProcessWorker(IocpEvent* iocpEvent, DWORD numOfBytes, SOCKET& listenSocket)
{
	switch (iocpEvent->eventType)
	{
	case EventType::Accept:
		cout << "Process Accept Start" << endl;
		if (false == SocketUtils::SetUpdateAcceptSocket(iocpEvent->_session->GetSocket(), listenSocket))
		{
			//RegisterAccept(acceptEvent);
			cout << "Process Accept Error" << endl;
			return;
		}
		break;

	case EventType::Connect:
		cout << "Process Connect Start" << endl;
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
