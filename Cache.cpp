#include "Cache.h"

#include <cmath>
#include <cstdint>
#include <iostream>

const int NUM_ADDRESS_BITS = 32;

Cache::Cache(int cacheSize, int associativity, int blockSize) {
    int numBlocks = cacheSize / blockSize;
    int numSets = numBlocks / associativity;

    int numOffsetBits = std::log2(blockSize);
    int numSetIdxBits = std::log2(numSets);
    int numTagBits = NUM_ADDRESS_BITS - numOffsetBits - numSetIdxBits;

    this->blkOffsetMask = 0xFFFFFFFF >> (NUM_ADDRESS_BITS - numOffsetBits);
    this->setIdxMask = 0xFFFFFFFF >> (NUM_ADDRESS_BITS - numSetIdxBits) << numOffsetBits;
    this->tagMask = 0xFFFFFFFF << (NUM_ADDRESS_BITS - numTagBits);

    std::cout << this->blkOffsetMask << std::endl
        << this->setIdxMask << std::endl
        << this->tagMask << std::endl;

    std::cout << "Creating cache..." << std::endl;
}

Cache::~Cache() {
    std::cout << "Destroying cache..." << std::endl;
}

int Cache::read(uint32_t address) {
    return -1;
}

int Cache::write(uint32_t address) {
    return -1;
}

