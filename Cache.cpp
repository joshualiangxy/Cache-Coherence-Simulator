#include "Cache.h"
#include "CacheLine.h"

#include <cmath>
#include <cstdint>
#include <iostream>

const int NUM_ADDRESS_BITS = 32;

Cache::Cache(int cacheSize, int associativity, int blockSize)
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

    this->cacheLines = std::vector<CacheLine>{numSets, CacheLine{associativity}};
}

Cache::~Cache() {}

int Cache::read(uint32_t address) {
    uint32_t setIdx = this->getSetIdx(address), tag = this->getTag(address);
    return this->cacheLines[setIdx].read(tag);
}

int Cache::write(uint32_t address) {
    uint32_t setIdx = this->getSetIdx(address), tag = this->getTag(address);
    return this->cacheLines[setIdx].write(tag);
}

uint32_t Cache::getSetIdx(uint32_t address) {
    return (address & this->setIdxMask) >> this->numOffsetBits;
}

uint32_t Cache::getTag(uint32_t address) {
    return (address & this->tagMask)
        >> (this->numOffsetBits + this->numSetIdxBits);
}

