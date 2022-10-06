#include "DragonCacheSet.h"

#include "Logger.h"

#include <stdexcept>
#include <utility>

DragonCacheSet::DragonCacheSet(int associativity): CacheSet{associativity} {}

DragonCacheSet::~DragonCacheSet() {};

void DragonCacheSet::read(uint32_t tag, std::shared_ptr<Logger> logger) {
    CacheSet::read(tag, logger);
    std::shared_ptr<CacheLineNode> node = this->cacheSet[tag];

    switch (node->state) {
        case CacheLineState::INVALID:
            break;
        case CacheLineState::EXCLUSIVE:
        case CacheLineState::MODIFIED:
            logger->incrementPrivateDataAccess();
            break;
        case CacheLineState::SHARED_CLEAN:
        case CacheLineState::SHARED_MODIFIED:
            logger->incrementPublicDataAccess();
            break;
        default:
            throw std::logic_error("Invalid cache state for Dragon");
    }
}

void DragonCacheSet::write(uint32_t tag, std::shared_ptr<Logger> logger) {
    CacheSet::write(tag, logger);
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
}

