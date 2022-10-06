#include "MESICacheSet.h"

#include "Logger.h"

#include <memory>
#include <stdexcept>
#include <utility>

MESICacheSet::MESICacheSet(int associativity): CacheSet{associativity} {}

MESICacheSet::~MESICacheSet() {};

void MESICacheSet::read(uint32_t tag, std::shared_ptr<Logger> logger) {
    CacheSet::read(tag, logger);
    std::shared_ptr<CacheLineNode> node = this->cacheSet[tag];

    switch (node->state) {
        case CacheLineState::INVALID:
            node->state = CacheLineState::SHARED;
            logger->incrementPublicDataAccess();
            break;
        case CacheLineState::SHARED:
            logger->incrementPublicDataAccess();
            break;
        case CacheLineState::EXCLUSIVE:
        case CacheLineState::MODIFIED:
            logger->incrementPrivateDataAccess();
            break;
        default:
            throw std::logic_error("Invalid cache state for MESI");
    }
}

void MESICacheSet::write(uint32_t tag, std::shared_ptr<Logger> logger) {
    CacheSet::write(tag, logger);
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
}

