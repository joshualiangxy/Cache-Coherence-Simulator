#ifndef GUARD_MESI_CACHE_SET_H
#define GUARD_MESI_CACHE_SET_H

#include "Bus.hpp"
#include "CacheSet.hpp"
#include "Logger.hpp"

#include <cstdint>
#include <memory>
#include <utility>

class MESICacheSet : public CacheSet {
public:
    MESICacheSet(
        int setIdx,
        int numSetIdxBits,
        int associativity,
        int blockSize
    );
    virtual ~MESICacheSet();

    int read(
        int threadID,
        uint32_t tag,
        std::shared_ptr<Bus> bus,
        std::shared_ptr<Logger> logger
    );
    int write(
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

