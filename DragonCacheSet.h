#ifndef GUARD_DRAGON_CACHE_SET_H
#define GUARD_DRAGON_CACHE_SET_H

#include "CacheSet.h"
#include "Logger.h"

#include <cstdint>
#include <memory>
#include <utility>

class DragonCacheSet : public CacheSet {
public:
    DragonCacheSet(int associativity);
    virtual ~DragonCacheSet();

    void read(uint32_t tag, std::shared_ptr<Logger> logger);
    void write(uint32_t tag, std::shared_ptr<Logger> logger);
};

#endif

