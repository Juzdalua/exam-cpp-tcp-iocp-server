#pragma once
#include "ClientPacketHandler.h"

extern unordered_map<uint16, uint16> packetIdToPriority;

struct ComparePacket
{
    bool operator()(BYTE* buffer1, BYTE* buffer2)
    {
        return packetIdToPriority[reinterpret_cast<PacketHeader*>(buffer1)->id] > packetIdToPriority[reinterpret_cast<PacketHeader*>(buffer2)->id];  // 높은 숫자가 우선순위가 높음
    }
};

class PacketPriorityQueue
{
public:
    void PushPacket(BYTE* buffer);
    void ProcessPackets(int32 len, GameProtobufSessionRef& session);

private:
    void HandlePacket(BYTE* buffer, int32 len, GameProtobufSessionRef& session);

    priority_queue<BYTE*, vector<BYTE*>, ComparePacket> _packetQueue;
    mutex _lock;
    condition_variable _cv;
};
