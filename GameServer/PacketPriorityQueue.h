#pragma once
#include "ClientPacketHandler.h"

extern unordered_map<uint16, uint16> packetIdToPriority;
extern class PacketPriorityQueue* GPacketPriorityQueue;

struct PacketData
{
    BYTE* buffer;
    int32 len;
    GameProtobufSessionRef session;
};

struct ComparePacket
{
    bool operator()(const PacketData& p1, const PacketData& p2) const
    {
        return packetIdToPriority[reinterpret_cast<PacketHeader*>(p1.buffer)->id] >
            packetIdToPriority[reinterpret_cast<PacketHeader*>(p2.buffer)->id];
    }
};

class PacketPriorityQueue
{
public:
    void PushPacket(BYTE* buffer, int32 len, GameProtobufSessionRef& session);
    void ProcessPackets();
    bool IsEmpty();

private:
    void HandlePacket(BYTE* buffer, int32 len, GameProtobufSessionRef& session);

    priority_queue<PacketData, vector<PacketData>, ComparePacket> _packetQueue;
    mutex _lock;
    condition_variable _cv;
};
