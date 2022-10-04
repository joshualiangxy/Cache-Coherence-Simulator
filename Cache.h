#ifndef GUARD_CACHE_H
#define GUARD_CACHE_H

#include <bitset>
#include <cstdint>

enum class CacheType { MESI, DRAGON };

class Cache {
public:
    Cache(int cacheSize, int associativity, int blockSize);
    virtual ~Cache();
    
    virtual int read(uint32_t address);
    virtual int write(uint32_t address);

private:
    std::bitset<32> blkOffsetMask;
    std::bitset<32> setIdxMask;
    std::bitset<32> tagMask;
};

#endif

