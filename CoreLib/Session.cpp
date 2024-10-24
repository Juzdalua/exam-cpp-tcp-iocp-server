#include "pch.h"
#include "Session.h"
#include "SocketUtils.h"
#include "Service.h"

Session::Session()
	:_recvBuffer(BUFFER_SIZE)
{
	_clientSocket = SocketUtils::CreateSocket();
}

Session::~Session()
{
	SocketUtils::Close(_clientSocket);
}

void Session::Send(SendBufferRef sendBuffer)
{
	if (IsConnected() == false)
		return;

	bool registerSend = false;

	// 현재 RegisterSend가 걸리지 않은 상태라면 걸어준다
	{
		//WRITE_LOCK;
		lock_guard<mutex> lock(_lock);
		_sendQueue.push(sendBuffer);
		if (_sendRegistered.exchange(true) == false)
			registerSend = true;
	}

	if (registerSend)
		RegisterSend();
}

// 서버-서버 연결
bool Session::Connect()
{
	return RegisterConnect();
}

void Session::Disconnect(const WCHAR* cause)
{
	if (_connected.exchange(false) == false)
		return;

	// TEMP
	DebugLog::PrintColorText(LogColor::YELLOW, "[Disconnect]", "", true, false);

	RegisterDisconnect();
}

HANDLE Session::GetHandle()
{
	return reinterpret_cast<HANDLE>(_clientSocket);
}

void Session::Dispatch(IocpEvent* iocpEvent, int32 numOfBytes)
{
	switch (iocpEvent->eventType)
	{
	case EventType::Connect:
		ProcessConnect();
		break;

	case EventType::Disconnect:
		ProcessDisconnect();
		break;

	case EventType::Recv:
		ProcessRecv(numOfBytes);
		break;

	case EventType::Send:
	{
		vector<SendBufferRef> sendVec = static_cast<SendEvent*>(iocpEvent)->sendBuffers;
		ProcessSend(numOfBytes, sendVec);
		break;
	}

	default:
		break;
	}
}

bool Session::RegisterConnect()
{
	if (IsConnected())
		return false;

	if (GetService()->GetServiceType() != ServiceType::Client)
		return false;

	if (SocketUtils::SetReuseAddress(_clientSocket, true) == false)
		return false;

	if (SocketUtils::BindAnyAddress(_clientSocket, 0/*남는 포트 아무거나*/) == false)
		return false;
	cout << "===== Set IOCP Done =====" << endl;

	_connectEvent.Init();
	_connectEvent.owner = shared_from_this();

	DWORD numOfBytes = 0;
	SOCKADDR_IN sockAddr = GetService()->GetNetAddress().GetSockAddr();
	if (SocketUtils::ConnectEx(_clientSocket, reinterpret_cast<SOCKADDR*>(&sockAddr), sizeof(sockAddr), nullptr, 0, &numOfBytes, &_connectEvent) == false)
	{
		int32 errorCode = WSAGetLastError();
		if (errorCode != WSA_IO_PENDING)
		{
			_connectEvent.owner = nullptr;
			return false;
		}
	}
	return true;
}

bool Session::RegisterDisconnect()
{
	_disconnectEvent.Init();
	_disconnectEvent.owner = shared_from_this();

	if (false == SocketUtils::DisconnectEx(_clientSocket, &_disconnectEvent, TF_REUSE_SOCKET, 0))
	{
		int32 errorCode = WSAGetLastError();
		if (errorCode != WSA_IO_PENDING)
		{
			_disconnectEvent.owner = nullptr;
			return false;
		}
	}

	return true;
}

void Session::RegisterRecv()
{
	if (IsConnected() == false)
		return;

	_recvEvent.Init();
	_recvEvent.owner = shared_from_this();

	WSABUF wsaBuf;
	wsaBuf.buf = reinterpret_cast<char*>(_recvBuffer.WritePos());
	wsaBuf.len = _recvBuffer.FreeSize();
	DWORD numOfBytes = 0;
	DWORD flags = 0;
	if (SOCKET_ERROR == WSARecv(_clientSocket, &wsaBuf, 1, OUT & numOfBytes, OUT & flags, &_recvEvent, nullptr))
	{
		int32 errorCode = WSAGetLastError();
		if (errorCode != WSA_IO_PENDING)
		{
			HandleError(errorCode);
			_recvEvent.owner = nullptr;
		}
	}
}

void Session::RegisterSend()
{
	if (IsConnected() == false)
		return;

	_sendEvent.Init();
	_sendEvent.owner = shared_from_this();

	// 보낼 데이터를 sendEvent에 등록
	{
		//WRITE_LOCK;
		lock_guard<mutex> lock(_lock);
		int32 writeSize = 0;
		while (_sendQueue.empty() == false)
		{
			SendBufferRef sendBuffer = _sendQueue.front();

			writeSize += sendBuffer->WriteSize();
			// TODO 예외체크 -> 센드버퍼보다 데이터 크기가 클 경우

			_sendQueue.pop();
			_sendEvent.sendBuffers.push_back(sendBuffer);
		}
	}

	// Scatter-Gather => 흩어져 있는 데이터들을 모아서 한방에 보낸다.
	vector<WSABUF> wsaBufs;
	wsaBufs.reserve(_sendEvent.sendBuffers.size());
	for (SendBufferRef sendBuffer : _sendEvent.sendBuffers)
	{
		WSABUF wsaBuf;
		wsaBuf.buf = reinterpret_cast<char*>(sendBuffer->Buffer());
		wsaBuf.len = static_cast<LONG>(sendBuffer->WriteSize());
		wsaBufs.push_back(wsaBuf);
	}

	DWORD numOfBytes = 0;
	if (SOCKET_ERROR == WSASend(_clientSocket, wsaBufs.data(), static_cast<DWORD>(wsaBufs.size()), OUT & numOfBytes, 0, &_sendEvent, nullptr))
	{
		int32 errorCode = WSAGetLastError();
		if (errorCode != WSA_IO_PENDING)
		{
			HandleError(errorCode);
			_sendEvent.owner = nullptr;	// shared_from_this RELEASE_REF
			_sendEvent.sendBuffers.clear(); // shared_from_this RELEASE_REF
			_sendRegistered.store(false);

		}
	}
}

void Session::ProcessConnect()
{
	_connectEvent.owner = nullptr;
	_connected.store(true);

	// 세션 등록
	GetService()->AddSession(GetSessionRef());

	// 컨텐츠 코드에서 오버로딩
	OnConnected();

	// 수신 등록
	RegisterRecv();
}

void Session::ProcessDisconnect()
{
	_disconnectEvent.owner = nullptr;

	OnDisconnected();
	GetService()->ReleaseSession(GetSessionRef());
}

void Session::ProcessRecv(int32 numOfBytes)
{
	_recvEvent.owner = nullptr;

	if (numOfBytes == 0)
	{
		Disconnect(L"Recv 0");
		return;
	}

	if (_recvBuffer.OnWrite(numOfBytes) == false)
	{
		Disconnect(L"OnWrite Overflow");
		return;
	}
	int32 dataSize = _recvBuffer.DataSize();
	int32 processLen = OnRecv(_recvBuffer.ReadPos(), dataSize);

	if (processLen < 0 || dataSize < processLen || _recvBuffer.OnRead(processLen) == false)
	{
		Disconnect(L"OnRead Overflow");
		return;
	}

	_recvBuffer.Clean();

	// 수신 재등록
	RegisterRecv();
}

void Session::ProcessSend(int32 numOfBytes, vector<SendBufferRef> sendVec)
{
	_sendEvent.owner = nullptr;	// shared_from_this RELEASE_REF
	_sendEvent.sendBuffers.clear(); // shared_from_this RELEASE_REF

	if (numOfBytes == 0)
	{
		Disconnect(L"Send 0");
		return;
	}

	//OnSend(numOfBytes);
	OnSend(numOfBytes, sendVec);

	//WRITE_LOCK;
	/*lock_guard<mutex> lock(_lock);
	if (_sendQueue.empty())
		_sendRegistered.store(false);
	else
		RegisterSend();*/

	{
		lock_guard<mutex> lock(_lock);
		if (_sendQueue.empty()) {
			_sendRegistered.store(false);
			return;
		}
	}

	RegisterSend();
}

void Session::HandleError(int32 errorCode)
{
	switch (errorCode)
	{
	case WSAECONNRESET:
	case WSAECONNABORTED:
		Disconnect(L"HandleError");
		break;

	default:
		// TODO: to log therad
		cout << "Handle Error: " << errorCode << endl;
		break;
	}
}

/*-----------------
	Packet Session
-----------------*/
PacketSession::PacketSession()
{
}

PacketSession::~PacketSession()
{
}

/*
	[header(4)] [data]
	[size(2)][id(2)][data]
	size = sizeof(data) + sizeof(header)
	-> dataSize = len - HeaderSize(4byte)

	10바이트의 데이터가 전송되면, [10][id][10byte data] -> 총 14byte가 전송된다.
*/
int32 PacketSession::OnRecv(BYTE* buffer, int32 len)
{
	int32 processLen = 0;

	/*if (processLen == 0)
	{
		int32 dataSize = len;
		if (dataSize < sizeof(PacketHeader))
			return;
		PacketHeader header = *(reinterpret_cast<PacketHeader*>(&buffer[0]));
		if (dataSize < header.size)
			return;
		OnRecvPacket(&buffer[0], header.size);
	}*/

	while (true)
	{
		int32 dataSize = len - processLen;

		// 최소 4바이트(헤더 크기)만큼은 있어야한다. => 헤더 파싱
		if (dataSize < sizeof(PacketHeader))
			break;

		// *((PacketHeader*)&buffer[0])
		PacketHeader header = *(reinterpret_cast<PacketHeader*>(&buffer[processLen]));

		// 헤더에 기록된 패킷 크기를 파싱할 수 있어야 한다.
		if (dataSize < header.size)
			break;

		// 패킷 조립 성공
		OnRecvPacket(&buffer[processLen], header.size);

		processLen += header.size;
	}

	return processLen;
}