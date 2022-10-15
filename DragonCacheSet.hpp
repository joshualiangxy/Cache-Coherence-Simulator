#ifndef GUARD_DRAGON_CACHE_SET_H
#define GUARD_DRAGON_CACHE_SET_H

#include "CacheSet.hpp"
#include "Logger.hpp"

#include <cstdint>
#include <memory>
#include <utility>

class DragonCacheSet : public CacheSet {
public:
    DragonCacheSet(
        int setIdx,
        int numSetIdxBits,
        int associativity,
        int blockSize
    );
    virtual ~DragonCacheSet();

    bool read(
        int threadID,
        uint32_t tag,
        std::shared_ptr<Bus> bus,
        std::shared_ptr<Logger> logger
    );
    bool write(
        int threadID,
        uint32_t tag,
        std::shared_ptr<Bus> bus,
        std::shared_ptr<Logger> logger
    );

    void handleBusReadEvent(uint32_t tag, std::shared_ptr<Logger> logger);
    void handleBusReadExclusiveEvent(
        int threadID,
        uint32_t tag,
        std::shared_ptr<Bus> bus,
        std::shared_ptr<Logger> logger
    );
    void handleBusUpdateEvent(uint32_t tag, std::shared_ptr<Logger> logger);
};

#endif

