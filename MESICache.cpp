#include "MESICache.h"

#include <iostream>

MESICache::MESICache(int cacheSize, int associativity, int blockSize)
        : Cache(cacheSize, associativity, blockSize) {
    std::cout << "Creating MESI cache..." << std::endl;
}

MESICache::~MESICache() {
    std::cout << "Destroying MESI cache..." << std::endl;
}

int MESICache::read(uint32_t address) {
    return -1;
}

int MESICache::write(uint32_t address) {
    return -1;
}

