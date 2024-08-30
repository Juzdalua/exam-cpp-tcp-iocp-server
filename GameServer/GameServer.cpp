#include "pch.h"
#include "ConnectionPool.h"

void worker(ConnectionPool& pool) {
    std::string query = "SELECT * FROM user";
    executeQuery(pool, query);
}

int main() {
    try {
        // 커넥션 풀을 생성합니다. 풀의 크기를 설정합니다.
        ConnectionPool pool(5);

        // 여러 개의 스레드를 생성하여 작업을 처리합니다.
        std::vector<std::thread> threads;
        for (int i = 0; i < 1; ++i) {
            threads.emplace_back(worker, std::ref(pool));
        }

        // 모든 스레드가 작업을 완료할 때까지 대기합니다.
        for (auto& t : threads) {
            if (t.joinable()) {
                t.join();
            }
        }
    }
    catch (const sql::SQLException& e) {
        std::cerr << "SQLException: " << e.what() << std::endl;
        std::cerr << "Error code: " << e.getErrorCode() << std::endl;
        std::cerr << "SQL state: " << e.getSQLState() << std::endl;
    }
    catch (const std::exception& e) {
        std::cerr << "Exception: " << e.what() << std::endl;
    }
    catch (...) {
        std::cerr << "Unknown exception occurred" << std::endl;
    }

    return 0;
}

//#include "SocketUtils.h"
//#include "IocpCore.h"
//#include "GameSession.h"
//#include "Service.h"
//#include "ENV.h"

//
//int32 MAX_CLIENT_COUNT = 1;
//int32 MAX_WORKER_COUNT = 2;
//
//int main()
//{
//	ENV::loadEnvFile("../CoreLib/.env");
//	string id = ENV::getEnvVariable("ID");
//	string pwd = ENV::getEnvVariable("PASSWORD");
//
//	SocketUtils::Init();
//
//	ServerServiceRef service = ServerServiceRef(
//		new ServerService(
//			NetAddress(L"127.0.0.1", 7777),
//			IocpCoreRef(new IocpCore()),
//			//[&]() {return shared_ptr<GameSession>(new GameSession());},
//			[&]() {return shared_ptr<GamePacketSession>(new GamePacketSession());},
//			MAX_CLIENT_COUNT
//		)
//	);
//
//	ASSERT_CRASH(service->Start());
//	
//	// Worker Threads
//	mutex m;
//	vector<thread> workers;
//	for (int32 i = 0; i < MAX_WORKER_COUNT; i++)
//	{
//		workers.push_back(thread([&]()
//			{
//				while (true) {
//					lock_guard<mutex> guard(m);
//					service->GetIocpCore()->Dispatch();
//				}
//			}));
//	}
//	cout << "===== Worker Thread Start =====" << endl;
//
//	for (thread& t : workers)
//	{
//		if (t.joinable())
//			t.join();
//	}
//	cout << "===== Worker Thread Exit =====" << endl;
//	SocketUtils::Clear();
//	cout << "===== Server has been shut down. =====" << endl;
//}
//
///*
//	Listener -> Socket Set -> Register Accept (AcceptEx) 
//	Process Accept -> Client Session Set -> Process Connect -> Register Recv (WSARecv)
//	Process Recv -> GameSession Echo Set -> Send with echo -> Register Recv (WSARecv)
//	SendBuffer Set -> Send (SendQueue Set) -> RegisterSend (WSASend) -> ProcessSend -> Register Send (WSASend)
//*/