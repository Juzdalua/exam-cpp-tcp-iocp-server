#include "pch.h"
#include "Service.h"
#include "IocpCore.h"
#include "SocketUtils.h"



class ServerSession : public Session
{
public:
	ServerSession()
	{
	}
	virtual ~ServerSession()
	{
	}

	virtual void OnConnected() override
	{
		cout << "===== Connected To Server =====" << endl;

		SendBufferRef sendBuffer = SendBufferRef(new SendBuffer(4096));
		char initSendData[] = "Ping";
		sendBuffer->CopyData(initSendData, sizeof(initSendData));
		Send(sendBuffer);
	}

	virtual int32 OnRecv(BYTE* buffer, int32 len) override
	{
		cout << "OnRecv Len = " << len << ", OnRecv Data = " << buffer << endl;
		this_thread::sleep_for(1s);

		return len;
	}

	virtual void OnSend(int32 len) override
	{
		cout << "OnSend Len = " << len << endl;
	}

	virtual void OnDisconnected() override
	{
		cout << "Disconnected" << endl;
	}
};

int main()
{
	this_thread::sleep_for(1s);
	SocketUtils::Init();

	ClientServiceRef service = ClientServiceRef(
		new ClientService(
			NetAddress(L"127.0.0.1", 7777),
			IocpCoreRef(new IocpCore()),
			[&]() {return shared_ptr<ServerSession>(new ServerSession());},
			1
		)
	);

	ASSERT_CRASH(service->Start());

	mutex m;
	vector<thread> workers;
	for (int32 i = 0; i < 2; i++)
	{
		workers.push_back(thread([&]()
			{
				while (true) {
					lock_guard<mutex> guard(m);
					service->GetIocpCore()->Dispatch();
				}
			}));
	}

	while (true)
	{
		char sendData[100];
		cin >> sendData;

		SendBufferRef sendBuffer = SendBufferRef(new SendBuffer(4096));
		sendBuffer->CopyData(sendData, sizeof(sendData));
		service->Broadcast(sendBuffer);
	}
	
	for (thread& t : workers)
	{
		if (t.joinable())
			t.join();
	}

	SocketUtils::Clear();
}