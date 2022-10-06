#include "MESICacheSet.h"

#include <stdexcept>
#include <utility>

MESICacheSet::MESICacheSet(int associativity): CacheSet{associativity} {}

MESICacheSet::~MESICacheSet() {};

std::pair<bool, int> MESICacheSet::read(uint32_t tag) {
    auto [isReadMiss, numCycles] = CacheSet::read(tag);
    std::shared_ptr<CacheLineNode> node = this->cacheSet[tag];

    switch (node->state) {
        case CacheLineState::INVALID:
            node->state = CacheLineState::SHARED;
            break;
        case CacheLineState::SHARED:
        case CacheLineState::EXCLUSIVE:
        case CacheLineState::MODIFIED:
            break;
        default:
            throw std::logic_error("Invalid cache state for MESI");
    }

    return { isReadMiss, numCycles };
}

std::pair<bool, int> MESICacheSet::write(uint32_t tag) {
    auto [isWriteMiss, numCycles] = CacheSet::write(tag);
    std::shared_ptr<CacheLineNode> node = this->cacheSet[tag];
    node->state = CacheLineState::MODIFIED;

    switch (node->state) {
        case CacheLineState::INVALID:
            break;
        case CacheLineState::SHARED:
            break;
        case CacheLineState::EXCLUSIVE:
        case CacheLineState::MODIFIED:
            break;
        default:
            throw std::logic_error("Invalid cache state for MESI");
    }

    return { isWriteMiss, numCycles };
}

