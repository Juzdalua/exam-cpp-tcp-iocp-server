#include "pch.h"
#include "PacketPriorityQueue.h"
#include "GameSession.h"

PacketPriorityQueue* GPacketPriorityQueue = nullptr;

unordered_map<uint16, uint16> packetIdToPriority =
{
    {PKT_C_HIT, 0},
    {PKT_C_SHOT, 1},
    {PKT_C_MOVE, 2},
};

void PacketPriorityQueue::PushPacket(BYTE* buffer, int32 len, GameProtobufSessionRef& session)
{
    lock_guard<mutex> lock(_lock);

    PacketData packetData = { buffer, len, session };
    _packetQueue.push(packetData);
    _cv.notify_one();
}

void PacketPriorityQueue::ProcessPackets()
{
    unique_lock<mutex> lock(_lock);

    while (!_packetQueue.empty())
    {
        PacketData packetData = _packetQueue.top();
        _packetQueue.pop();

        HandlePacket(packetData.buffer, packetData.len, packetData.session);
    }
}

bool PacketPriorityQueue::IsEmpty()
{
    lock_guard<mutex> lock(_lock);
    return _packetQueue.empty();
}

void PacketPriorityQueue::HandlePacket(BYTE* buffer, int32 len, GameProtobufSessionRef& session)
{
    PacketHeader* recvHeader = reinterpret_cast<PacketHeader*>(buffer);

    HandlePacketStartLog("RECV", LogColor::GREEN, recvHeader, session);

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

