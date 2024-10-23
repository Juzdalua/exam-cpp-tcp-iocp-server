#include "pch.h"
#include "ThreadQueue.h"
#include "../GameServer/pch.h"

ThreadQueue<PacketData> packetQueue; // IOCP -> Packet
ThreadQueue<function<void()>> dbQueue; // Packet -> DB
ThreadQueue<SendData> sendQueue; // Packet -> Send
ThreadQueue<LogData> logQueue; // Packet -> Log