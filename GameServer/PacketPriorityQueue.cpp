#include "pch.h"
#include "PacketPriorityQueue.h"

unordered_map<uint16, uint16> packetIdToPriority =
{
    {PKT_C_HIT, 0},
    {PKT_C_SHOT, 1},
    {PKT_C_MOVE, 2},
};

void PacketPriorityQueue::PushPacket(BYTE* buffer)
{
    lock_guard<mutex> lock(_lock);
    _packetQueue.push(buffer);
    _cv.notify_one();
}

void PacketPriorityQueue::ProcessPackets(int32 len, GameProtobufSessionRef& session)
{
    while (true)
    {
        unique_lock<mutex> lock(_lock);
        _cv.wait(lock, [this]() { return !_packetQueue.empty(); });

        while (!_packetQueue.empty())
        {
            BYTE* buffer = _packetQueue.top();
            _packetQueue.pop();
            lock.unlock();

            // 패킷 처리 로직
            HandlePacket(buffer, len, session);
            lock.lock();
        }
    }
}

void PacketPriorityQueue::HandlePacket(BYTE* buffer, int32 len, GameProtobufSessionRef& session)
{
    PacketHeader* recvHeader = reinterpret_cast<PacketHeader*>(buffer);
    switch (recvHeader->id)
    {
    case PKT_C_MOVE:
        ClientPacketHandler::HandleMove(buffer, len, session);
        break;

    case PKT_C_SHOT:
        ClientPacketHandler::HandleShot(buffer, len, session);
        break;

    case PKT_C_HIT:
        ClientPacketHandler::HandleHit(buffer, len, session);
        break;

    default:
        break;
    }
}

