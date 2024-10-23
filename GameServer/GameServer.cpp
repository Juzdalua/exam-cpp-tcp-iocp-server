#include "pch.h"
#include "IocpCore.h"
#include "GameSession.h"
#include "Service.h"
#include "Protocol.pb.h"
#include "ThreadQueue.h"
#include "ClientPacketHandler.h"

CoreGlobal GCoreGlobal;

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

void SendWorker()
{

}

void LogWorker()
{

}

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
	cout << "===== Worker Thread Exit =====" << endl;

	for (thread& t : workers)
	{
		if (t.joinable())
			t.join();
	}
	cout << "===== Server has been shut down. =====" << endl;
}

/*
	Listener -> Socket Set -> Register Accept (AcceptEx)
	Process Accept -> Client Session Set -> Process Connect -> Register Recv (WSARecv)
	Process Recv -> GameSession Echo Set -> Send with echo -> Register Recv (WSARecv)
	SendBuffer Set -> Send (SendQueue Set) -> RegisterSend (WSASend) -> ProcessSend -> Register Send (WSASend)
*/