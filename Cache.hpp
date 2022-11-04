#ifndef GUARD_CACHE_H
#define GUARD_CACHE_H

#include "Bus.hpp"
#include "CacheSet.hpp"
#include "Logger.hpp"

#include <cstdint>
#include <memory>
#include <unordered_set>
#include <utility>
#include <vector>

enum class CacheType { MESI, DRAGON };

class Cache {
public:
    Cache(
        int threadID,
        int cacheSize,
        int associativity,
        int blockSize,
        CacheType cacheType,
        std::shared_ptr<Logger> logger
    );

    int read(uint32_t address, std::shared_ptr<Bus> bus);
    int write(uint32_t address, std::shared_ptr<Bus> bus);

    void handleBusEvents(
        std::queue<BusEvent>& eventQueue,
        std::shared_ptr<Bus> bus
    );

private:
    int threadID;
    int numOffsetBits;
    int numSetIdxBits;
    int numTagBits;
    int associativity;
    std::vector<std::shared_ptr<CacheSet>> cacheSets;
    std::unordered_set<uint32_t> uniqueBlockSet;

    std::shared_ptr<Logger> logger;
    uint32_t blkOffsetMask;
    uint32_t setIdxMask;
    uint32_t tagMask;

    uint32_t getSetIdx(uint32_t address);
    uint32_t getTag(uint32_t address);
    uint32_t getBlockIdx(uint32_t address);

    void handleBusReadEvent(uint32_t blockIdx);
    void handleBusReadExclusiveEvent(uint32_t blockIdx, std::shared_ptr<Bus> bus);
    void handleBusUpdateEvent(uint32_t blockIdx);
};

#endif

