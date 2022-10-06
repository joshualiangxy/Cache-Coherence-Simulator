#ifndef GUARD_DRAGON_CACHE_SET_H
#define GUARD_DRAGON_CACHE_SET_H

#include "CacheSet.h"

#include <cstdint>
#include <utility>

class DragonCacheSet : public CacheSet {
public:
    DragonCacheSet(int associativity);
    virtual ~DragonCacheSet();

    std::pair<bool, int> read(uint32_t tag);
    std::pair<bool, int> write(uint32_t tag);
};

#endif

