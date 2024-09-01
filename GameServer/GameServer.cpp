//#include "pch.h"
//#include <openssl/sha.h>
//#include <iostream>
//#include <iomanip>
//#include <sstream>
//#include <string>
//
//// 문자열을 SHA-256 해시로 변환하는 함수
//std::string ComputeSha256Hash(const std::string& data) {
//	unsigned char hash[SHA256_DIGEST_LENGTH];
//	SHA256_CTX sha256;
//	SHA256_Init(&sha256);
//	SHA256_Update(&sha256, data.c_str(), data.size());
//	SHA256_Final(hash, &sha256);
//
//	std::stringstream ss;
//	for (int i = 0; i < SHA256_DIGEST_LENGTH; ++i) {
//		ss << std::hex << std::setw(2) << std::setfill('0') << (int)hash[i];
//	}
//	return ss.str();
//}
//
//int main() {
//	// 클라이언트에서 전달된 해시
//	std::string receivedHash = "클라이언트에서 받은 해시 문자열";
//
//	// 서버에서 저장된 원본 비밀번호
//	std::string storedPassword = "your_password";
//
//	// 서버에서 비밀번호 해시 생성
//	std::string computedHash = ComputeSha256Hash(storedPassword);
//
//	// 클라이언트 해시와 비교
//	if (computedHash == receivedHash) {
//		std::cout << "Authentication successful!" << std::endl;
//	}
//	else {
//		std::cout << "Authentication failed!" << std::endl;
//	}
//
//	return 0;
//}
//





#include "pch.h"
#include "ConnectionPool.h"
#include "SocketUtils.h"
#include "IocpCore.h"
#include "GameSession.h"
#include "Service.h"

CoreGlobal GCoreGlobal;
int32 MAX_CLIENT_COUNT = 1;
int32 MAX_WORKER_COUNT = 2;

int main()
{
	/*unique_ptr<sql::ResultSet> result = executeQuery(*CP, "Select * FROM user;");
	while (result->next()) {
		cout << result->getString(1) << " " << result->getString(2) << endl;
	}*/

	SocketUtils::Init();

	ServerServiceRef service = ServerServiceRef(
		new ServerService(
			NetAddress(L"127.0.0.1", 7777),
			IocpCoreRef(new IocpCore()),
			//[&]() {return shared_ptr<GameSession>(new GameSession());},
			//[&]() {return shared_ptr<GamePacketSession>(new GamePacketSession());},
			[&]() {return shared_ptr<GameProtobufSession>(new GameProtobufSession());},
			MAX_CLIENT_COUNT
		)
	);

	ASSERT_CRASH(service->Start());

	// Worker Threads
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
	cout << "===== Worker Thread Start =====" << endl;

	for (thread& t : workers)
	{
		if (t.joinable())
			t.join();
	}
	cout << "===== Worker Thread Exit =====" << endl;
	SocketUtils::Clear();
	cout << "===== Server has been shut down. =====" << endl;
}

/*
	Listener -> Socket Set -> Register Accept (AcceptEx)
	Process Accept -> Client Session Set -> Process Connect -> Register Recv (WSARecv)
	Process Recv -> GameSession Echo Set -> Send with echo -> Register Recv (WSARecv)
	SendBuffer Set -> Send (SendQueue Set) -> RegisterSend (WSASend) -> ProcessSend -> Register Send (WSASend)
*/