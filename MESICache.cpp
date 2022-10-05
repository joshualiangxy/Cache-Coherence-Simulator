#include "MESICache.h"

#include <iostream>

MESICache::MESICache(int cacheSize, int associativity, int blockSize)
        : Cache(cacheSize, associativity, blockSize) {}

MESICache::~MESICache() {}

int MESICache::read(uint32_t address) {
    return Cache::read(address);
}

int MESICache::write(uint32_t address) {
    return Cache::read(address);
}

