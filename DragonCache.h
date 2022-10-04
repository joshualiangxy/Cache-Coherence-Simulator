#ifndef GUARD_DRAGON_CACHE_H
#define GUARD_DRAGON_CACHE_H

#include "Cache.h"

#include <cstdint>

class DragonCache : public Cache {
public:
    DragonCache(int cacheSize, int associativity, int blockSize);
    virtual ~DragonCache();

    int read(uint32_t address);
    int write(uint32_t address);
};

#endif

