#include "pch.h"
#include "IocpCore.h"
#include "IocpEvent.h"

IocpCore::IocpCore()
{
	_iocpHandle = CreateIoCompletionPort(INVALID_HANDLE_VALUE, 0, 0, 0);
	ASSERT_CRASH(_iocpHandle != INVALID_HANDLE_VALUE);
}

IocpCore::~IocpCore()
{
	CloseHandle(_iocpHandle);
}

bool IocpCore::Register(HANDLE iocpHandle)
{
	return CreateIoCompletionPort(iocpHandle, _iocpHandle, /*key*/0, 0);
}

void IocpCore::Dispatch(uint32 timeoutMs)
{
	DWORD numOfBytes = 0;
	ULONG_PTR key = 0;
	IocpEvent* iocpEvent = nullptr;

	if (GetQueuedCompletionStatus(_iocpHandle, OUT &numOfBytes, OUT &key, OUT reinterpret_cast<LPOVERLAPPED*>(&iocpEvent), timeoutMs))
	{
		shared_ptr<IocpObject> iocpObject = iocpEvent->owner;
		iocpObject->Dispatch(iocpEvent, numOfBytes);
		
		//iocpEvent->eventType;
	}
	else
	{
		int32 errCode = WSAGetLastError();

		switch (errCode)
		{
		case WAIT_TIMEOUT:
			return;
		default:
			// TODO: ·Î±× Âï±â
			//IocpObjectRef iocpObject = iocpEvent->owner;
			//iocpObject->Dispatch(iocpEvent, numOfBytes);
			break;
		}
	}
}

void IocpCore::RegisterAccept(IocpEvent* iocpEvent)
{
	if (iocpEvent->eventType != EventType::Accept)
		return;
}

void IocpCore::RegisterRecv(IocpEvent* iocpEvent)
{
	if (iocpEvent->eventType != EventType::Recv)
		return;

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

void IocpCore::RegisterSend(IocpEvent* iocpEvent)
{
	if (iocpEvent->eventType != EventType::Send)
		return;
}
