#include "pch.h"
#include <queue>
#include <mutex>
#include <condition_variable>
#include <functional>
#include "../GameServer/pch.h"

struct PacketData
{
    BYTE* buffer;
    int32 len;
    GameProtobufSessionRef session;
};

template<typename T>
class ThreadQueue 
 {
public:
    void Push(T value) {
        lock_guard<mutex> lock(_mutex);
        _queue.push(move(value));
        _condition.notify_one();
    }

    bool Pop(T& result) {
        unique_lock<mutex> lock(_mutex);
        _condition.wait(lock, [this] { return !_queue.empty(); });

        if (!_queue.empty()) {
            result = move(_queue.front());
            _queue.pop();
            return true;
        }
        return false;
    }

    uint64 GetSize() { return _queue.size(); }

private:
    queue<T> _queue;
    mutex _mutex;
    condition_variable _condition;
};

extern ThreadQueue<PacketData> packetQueue;