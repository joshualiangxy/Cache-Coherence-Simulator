#include "Bus.hpp"

#include "Cache.hpp"

#include <memory>
#include <mutex>

const int NUM_THREADS = 4;

BusEvent::BusEvent(BusEventType type, uint32_t blockIdx)
    : type{type}
    , blockIdx{blockIdx} {}

BusEvent::BusEvent(const BusEvent& toCopy)
    : type{toCopy.type}
    , blockIdx{toCopy.blockIdx} {}

Bus::Bus(): threadMutexes{NUM_THREADS}, eventQueues{NUM_THREADS} {}

bool Bus::busReadAndCheckIsExclusive(uint32_t blockIdx, int threadID) {
    bool isExclusive;
    {
        std::scoped_lock lock{this->busMutex};

        isExclusive = this->blockIdxCountMap[blockIdx].empty();
        this->blockIdxCountMap[blockIdx].insert(threadID);
    }

    this->sendEvent(blockIdx, BusEventType::BUS_READ, threadID);
    return isExclusive;
}

bool Bus::busReadExclusiveAndCheckIsExclusive(uint32_t blockIdx, int threadID) {
    bool isExclusive;
    {
        std::scoped_lock lock{this->busMutex};

        isExclusive = this->blockIdxCountMap[blockIdx].empty();
        this->blockIdxCountMap[blockIdx].insert(threadID);
    }

    this->sendEvent(blockIdx, BusEventType::BUS_READ_EXCLUSIVE, threadID);
    return isExclusive;
}

bool Bus::busUpdateAndCheckIsExclusive(uint32_t blockIdx, int threadID) {
    bool isExclusive;
    {
        std::scoped_lock lock{this->busMutex};

        isExclusive = this->blockIdxCountMap[blockIdx].empty();
        this->blockIdxCountMap[blockIdx].insert(threadID);
    }

    this->sendEvent(blockIdx, BusEventType::BUS_UPDATE, threadID);
    return isExclusive;
}

void Bus::invalidateBlock(uint32_t blockIdx, int threadID) {
    std::scoped_lock lock{this->busMutex};
    this->blockIdxCountMap[blockIdx].erase(threadID);
}

void Bus::sendEvent(uint32_t blockIdx, BusEventType eventType, int threadID) {
    for (int otherThreadID = 0; otherThreadID < NUM_THREADS; ++otherThreadID) {
        if (threadID == otherThreadID) continue;

        { 
            std::scoped_lock lock{this->threadMutexes[otherThreadID]};
            this->eventQueues[otherThreadID].emplace(eventType, blockIdx);
        }
    }
}

std::queue<BusEvent> Bus::getEventsInQueue(int threadID) {
    std::scoped_lock lock{this->threadMutexes[threadID]};

    std::queue<BusEvent> eventQueue = this->eventQueues[threadID];
    this->eventQueues[threadID] = std::queue<BusEvent>();

    return eventQueue;
}

bool Bus::hasNodeInForwardState() {
    std::scoped_lock lock{this->busMutex};
    return this->hasForwardState;
}

void Bus::setHasForwardState(bool state) {
    std::scoped_lock lock{this->busMutex};
    this->hasForwardState = state;
}

