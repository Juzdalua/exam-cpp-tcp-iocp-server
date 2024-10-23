#include "pch.h"
#include "IocpCore.h"
#include "GameSession.h"
#include "Service.h"
#include "Protocol.pb.h"
#include "ThreadQueue.h"
#include "ClientPacketHandler.h"

CoreGlobal GCoreGlobal;
void IocpWorker(ServerServiceRef& service);
void PacketWorker();
void DBWorker();
void SendWorker();
void LogWorker();

int main()
{
	ServerServiceRef service = ServerServiceRef(
		new ServerService(
			NetAddress(L"127.0.0.1", 7777),
			IocpCoreRef(new IocpCore()),
			[&]() {return GameProtobufSessionRef(new GameProtobufSession());},
			MAX_CLIENT_COUNT
		)
	);

	ASSERT_CRASH(service->Start());

	// Worker Threads
	cout << "===== Worker Thread Start =====" << endl;
	vector<thread> workers;
	workers.push_back(thread([&]() {
		IocpWorker(service);
		}));
	workers.push_back(thread([&]() {
		PacketWorker();
		}));
	workers.push_back(thread([&]() {
		DBWorker();
		}));
	workers.push_back(thread([&]() {
		SendWorker();
		}));
	workers.push_back(thread([&]() {
		LogWorker();
		}));
	cout << "===== Worker Thread Exit =====" << endl;

	for (thread& t : workers)
	{
		if (t.joinable())
			t.join();
	}
	cout << "===== Server has been shut down. =====" << endl;
}

// Worker Thread
void IocpWorker(ServerServiceRef& service)
{
	while (true)
	{
		service->GetIocpCore()->Dispatch(10);
	}
}

void PacketWorker()
{
	while (true) {
		PacketData pkt;
		packetQueue.Pop(pkt);

		PacketHeader* recvHeader = reinterpret_cast<PacketHeader*>(pkt.buffer);
		HandlePacketStartLog("RECV", LogColor::GREEN, recvHeader, pkt.session);
		ClientPacketHandler::HandlePacket(pkt.buffer, pkt.len, pkt.session);
	}
}

void DBWorker()
{
	while (true)
	{
		/*SendData pkt;
		dbQueue.Pop(pkt);*/
	}
}

void SendWorker()
{
	using Clock = chrono::steady_clock;

	while (true)
	{
		auto start = Clock::now();

		SendData pkt;
		sendQueue.Pop(pkt);

		if (pkt.sendType == SendType::Broadcast)
		{
			GRoom.Broadcast(pkt.sendBuffer);
		}
		else if (pkt.sendType == SendType::Send && pkt.session != nullptr)
		{
			pkt.session->Send(pkt.sendBuffer);
		}

		auto end = Clock::now();
		auto elapsed = chrono::duration_cast<chrono::milliseconds>(end - start);
		
		auto sleepTime = chrono::milliseconds(FPS) - elapsed;
		if (sleepTime.count() > 0)
			this_thread::sleep_for(sleepTime);
	}
}

void LogWorker()
{
	while (true)
	{
		LogData pkt;
		logQueue.Pop(pkt);
		string logType = "";

		if (pkt.logType == LogType::CONNECT)
			logType = "Client Connected";
		else if (pkt.logType == LogType::DISCONNECT)
			logType = "Disconnected";
		else if (pkt.logType == LogType::SEND)
			logType = "SEND";
		else if (pkt.logType == LogType::RECV)
			logType = "RECV";

		HandlePacketStartLog(logType, pkt.color, pkt.recvHeader, pkt.session);
	}
}


/*
	Listener -> Socket Set -> Register Accept (AcceptEx)
	Process Accept -> Client Session Set -> Process Connect -> Register Recv (WSARecv)
	Process Recv -> GameSession Echo Set -> Send with echo -> Register Recv (WSARecv)
	SendBuffer Set -> Send (SendQueue Set) -> RegisterSend (WSASend) -> ProcessSend -> Register Send (WSASend)
*/