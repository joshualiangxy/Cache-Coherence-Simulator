#ifndef GUARD_CACHE_H
#define GUARD_CACHE_H

#include "CacheSet.h"

#include <cstdint>
#include <memory>
#include <utility>
#include <vector>

enum class CacheType { MESI, DRAGON };

class Cache {
public:
    Cache(int cacheSize, int associativity, int blockSize, CacheType cacheType);
    
    std::pair<bool, int> read(uint32_t address);
    std::pair<bool, int> write(uint32_t address);

private:
    uint32_t blkOffsetMask;
    uint32_t setIdxMask;
    uint32_t tagMask;

    int numOffsetBits;
    int numSetIdxBits;
    int numTagBits;
    int associativity;
    std::vector<std::shared_ptr<CacheSet>> cacheSets;

    uint32_t getSetIdx(uint32_t address);
    uint32_t getTag(uint32_t address);
};

#endif

