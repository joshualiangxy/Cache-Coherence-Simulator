#ifndef GUARD_MESI_CACHE_H
#define GUARD_MESI_CACHE_H

#include "Cache.h"

#include <cstdint>

class MESICache : public Cache {
public:
    MESICache(int cacheSize, int associativity, int blockSize);
    virtual ~MESICache();

    virtual int read(uint32_t address);
    virtual int write(uint32_t address);
};

#endif

