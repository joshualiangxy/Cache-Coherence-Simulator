#ifndef GUARD_MESIF_CACHE_SET_H
#define GUARD_MESIF_CACHE_SET_H

#include "Bus.hpp"
#include "CacheSet.hpp"
#include "Logger.hpp"

#include <cstdint>
#include <memory>
#include <utility>

class MESIFCacheSet : public CacheSet {
    public:
        MESIFCacheSet(
            int setIdx,
            int numSetIdxBits,
            int associativity,
            int blockSize
        );
        virtual ~MESIFCacheSet();

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

        void handleBusReadEvent(
            uint32_t tag, 
            std::shared_ptr<Bus> bus,
            std::shared_ptr<Logger> logger
        );
        void handleBusReadExclusiveEvent(
            int threadID,
            uint32_t tag,
            std::shared_ptr<Bus> bus,
            std::shared_ptr<Logger> logger
        );
        void handleBusUpdateEvent(uint32_t tag, std::shared_ptr<Logger> logger);
};

#endif