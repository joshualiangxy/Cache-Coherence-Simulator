#include "Cache.h"
#include "CacheSet.h"
#include "MESICacheSet.h"
#include "DragonCacheSet.h"

#include <cmath>
#include <cstdint>

const int NUM_ADDRESS_BITS = 32;

Cache::Cache(int cacheSize, int associativity, int blockSize, CacheType cacheType)
        : associativity{associativity} {
    int numBlocks = cacheSize / blockSize;
    int numSets = numBlocks / associativity;

    this->numOffsetBits = std::log2(blockSize);
    this->numSetIdxBits = std::log2(numSets);
    this->numTagBits = NUM_ADDRESS_BITS - numOffsetBits - numSetIdxBits;

    this->blkOffsetMask = 0xFFFFFFFF >> (NUM_ADDRESS_BITS - this->numOffsetBits);
    this->setIdxMask = 0xFFFFFFFF >> (NUM_ADDRESS_BITS - this->numSetIdxBits)
        << this->numOffsetBits;
    this->tagMask = 0xFFFFFFFF << (NUM_ADDRESS_BITS - this->numTagBits);

    this->cacheSets.reserve(numSets);
    for (int i = 0; i < numSets; ++i) {
        switch (cacheType) {
            case CacheType::MESI:
                this->cacheSets.emplace_back(
                    std::make_shared<MESICacheSet>(associativity)
                );
                break;

            case CacheType::DRAGON:
                this->cacheSets.emplace_back(
                    std::make_shared<DragonCacheSet>(associativity)
                );
                break;
        }
    }
}

std::pair<bool, int> Cache::read(uint32_t address) {
    uint32_t setIdx = this->getSetIdx(address), tag = this->getTag(address);
    return this->cacheSets[setIdx]->read(tag);
}

std::pair<bool, int> Cache::write(uint32_t address) {
    uint32_t setIdx = this->getSetIdx(address), tag = this->getTag(address);
    return this->cacheSets[setIdx]->write(tag);
}

uint32_t Cache::getSetIdx(uint32_t address) {
    return (address & this->setIdxMask) >> this->numOffsetBits;
}

uint32_t Cache::getTag(uint32_t address) {
    return (address & this->tagMask)
        >> (this->numOffsetBits + this->numSetIdxBits);
}

