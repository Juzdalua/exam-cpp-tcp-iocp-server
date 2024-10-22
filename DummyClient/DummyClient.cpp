#include "pch.h"
#include "Service.h"
#include "SocketUtils.h"
#include "ServerSession.h"
#include "Protocol.pb.h"

int32 MAX_WORKER_COUNT = 2;
int32 CLIENT_COUNT = 1;

int main()
{
	this_thread::sleep_for(1s);
	SocketUtils::Init();

	cout << "추가할 클라이언트 수를 입력하세요 (0 입력 시 종료): ";
	cin >> CLIENT_COUNT;

	ClientServiceRef service = ClientServiceRef(
		new ClientService(
			NetAddress(L"127.0.0.1", 7777),
			IocpCoreRef(new IocpCore()),
			[&]() {return shared_ptr<ServerProtobufSession>(new ServerProtobufSession());},
			CLIENT_COUNT
		)
	);

	ASSERT_CRASH(service->Start());

	mutex m;
	vector<thread> workers;
	for (int32 i = 0; i < MAX_WORKER_COUNT; i++)
	{
		workers.push_back(thread([&]()
			{
				while (true) {
					lock_guard<mutex> guard(m);
					service->GetIocpCore()->Dispatch();
				}
			}));
	}

	for (thread& t : workers)
	{
		if (t.joinable())
			t.join();
	}

	SocketUtils::Clear();
}