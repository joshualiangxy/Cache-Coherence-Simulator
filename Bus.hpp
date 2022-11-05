#ifndef GUARD_BUS_H
#define GUARD_BUS_H

#include <cstdint>
#include <mutex>
#include <queue>
#include <unordered_map>
#include <unordered_set>
#include <vector>

enum class BusEventType {
    BUS_READ,
    BUS_READ_EXCLUSIVE,
    BUS_UPDATE
};

struct BusEvent {
    BusEventType type;
    uint32_t blockIdx;

    BusEvent(BusEventType type, uint32_t blockIdx);
    BusEvent(const BusEvent& toCopy);
};

class Bus {
public:
    Bus();

    bool busReadAndCheckIsExclusive(uint32_t blockIdx, int threadID);
    bool busReadExclusiveAndCheckIsExclusive(uint32_t blockIdx, int threadID);
    bool busUpdateAndCheckIsExclusive(uint32_t blockIdx, int threadID);

    bool canFetchFromAnotherNode(uint32_t blockIdx);
    void incrementBlockIdxShareableCount(uint32_t blockIdx);
    void decrementBlockIdxShareableCount(uint32_t blockIdx);
    

    void invalidateBlock(uint32_t blockIdx, int threadID);


    std::queue<BusEvent> getEventsInQueue(int threadID);

private:
    std::mutex busMutex;
    std::vector<std::mutex> threadMutexes;

    std::unordered_map<uint32_t, std::unordered_set<int>> blockIdxCountMap;
    std::vector<std::queue<BusEvent>> eventQueues;

    std::unordered_map<uint32_t, int> blockIdxCountCacheShareable;

    void sendEvent(uint32_t blockIdx, BusEventType eventType, int threadID);
};

#endif

