#include "DragonCache.h"

#include <cstdint>
#include <iostream>

DragonCache::DragonCache(int cacheSize, int associativity, int blockSize)
        : Cache(cacheSize, associativity, blockSize) {}

DragonCache::~DragonCache() {}

int DragonCache::read(uint32_t address) {
    return Cache::read(address);
}

int DragonCache::write(uint32_t address) {
    return Cache::write(address);
}

