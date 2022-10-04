#include "DragonCache.h"

#include <cstdint>
#include <iostream>

DragonCache::DragonCache(int cacheSize, int associativity, int blockSize)
        : Cache(cacheSize, associativity, blockSize) {
    std::cout << "Creating Dragon cache..." << std::endl;
}

DragonCache::~DragonCache() {
    std::cout << "Destroying Dragon cache..." << std::endl;
}

int DragonCache::read(uint32_t address) {
    return -1;
}

int DragonCache::write(uint32_t address) {
    return -1;
}

