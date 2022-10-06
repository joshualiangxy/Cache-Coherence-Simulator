#include "DragonCacheSet.h"

#include <stdexcept>
#include <utility>

DragonCacheSet::DragonCacheSet(int associativity): CacheSet{associativity} {}

DragonCacheSet::~DragonCacheSet() {};

std::pair<bool, int> DragonCacheSet::read(uint32_t tag) {
    auto [isReadMiss, numCycles] = CacheSet::read(tag);
    std::shared_ptr<CacheLineNode> node = this->cacheSet[tag];

    switch (node->state) {
        case CacheLineState::INVALID:
            break;
        case CacheLineState::EXCLUSIVE:
        case CacheLineState::MODIFIED:
            break;
        case CacheLineState::SHARED_CLEAN:
        case CacheLineState::SHARED_MODIFIED:
            break;
        default:
            throw std::logic_error("Invalid cache state for Dragon");
    }

    return { isReadMiss, numCycles };
}

std::pair<bool, int> DragonCacheSet::write(uint32_t tag) {
    auto [isWriteMiss, numCycles] = CacheSet::write(tag);
    std::shared_ptr<CacheLineNode> node = this->cacheSet[tag];

    switch (node->state) {
        case CacheLineState::INVALID:
            break;
        case CacheLineState::EXCLUSIVE:
            node->state = CacheLineState::MODIFIED;
            break;
        case CacheLineState::MODIFIED:
            break;
        case CacheLineState::SHARED_CLEAN:
            break;
        case CacheLineState::SHARED_MODIFIED:
            break;
        default:
            throw std::logic_error("Invalid cache state for Dragon");
    }

    return { isWriteMiss, numCycles };
}

