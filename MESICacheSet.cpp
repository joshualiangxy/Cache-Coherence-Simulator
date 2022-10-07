#include "MESICacheSet.hpp"

#include "Logger.hpp"

#include <memory>
#include <stdexcept>
#include <utility>

MESICacheSet::MESICacheSet(int setIdx, int numSetIdxBits, int associativity)
    : CacheSet{setIdx, numSetIdxBits, associativity} {}

MESICacheSet::~MESICacheSet() {};

void MESICacheSet::read(
    int threadID,
    uint32_t tag,
    std::shared_ptr<Bus> bus,
    std::shared_ptr<Logger> logger
) {
    CacheSet::read(threadID, tag, bus, logger);
    std::shared_ptr<CacheLineNode> node = this->cacheSet[tag];

    switch (node->state) {
        case CacheLineState::INVALID: {
            bool isExclusive = bus->busReadAndCheckIsExclusive(
                this->getBlockIdx(tag),
                threadID
            );
            node->state = isExclusive
                ? CacheLineState::EXCLUSIVE
                : CacheLineState::SHARED;
            break;
        }
        case CacheLineState::SHARED:
        case CacheLineState::EXCLUSIVE:
        case CacheLineState::MODIFIED:
            break;
        default:
            throw std::logic_error("Invalid cache state for MESI");
    }

    if (node->state == CacheLineState::EXCLUSIVE
            || node->state == CacheLineState::MODIFIED) {
        logger->incrementPrivateDataAccess();
    } else {
        logger->incrementPublicDataAccess();
    }
}

void MESICacheSet::write(
    int threadID,
    uint32_t tag,
    std::shared_ptr<Bus> bus,
    std::shared_ptr<Logger> logger
) {
    CacheSet::write(threadID, tag, bus, logger);
    std::shared_ptr<CacheLineNode> node = this->cacheSet[tag];

    switch (node->state) {
        case CacheLineState::INVALID:
        case CacheLineState::SHARED:
            bus->busReadExclusive(this->getBlockIdx(tag), threadID);
            break;
        case CacheLineState::EXCLUSIVE:
        case CacheLineState::MODIFIED:
            break;
        default:
            throw std::logic_error("Invalid cache state for MESI");
    }

    node->state = CacheLineState::MODIFIED;
}

void MESICacheSet::handleBusReadEvent(
    uint32_t tag,
    std::shared_ptr<Logger> logger
) {
    if (!this->cacheSet.contains(tag)) return;

    std::shared_ptr<CacheLineNode> node = this->cacheSet[tag];

    switch (node->state) {
        case CacheLineState::SHARED:
        case CacheLineState::EXCLUSIVE:
        case CacheLineState::MODIFIED:
            node->state = CacheLineState::SHARED;
            break;
        case CacheLineState::INVALID:
            throw std::logic_error("Invalid cache state for BusRd in MESI");
        default:
            throw std::logic_error("Invalid cache state for MESI");
    }
}

void MESICacheSet::handleBusReadExclusiveEvent(
    int threadID,
    uint32_t tag,
    std::shared_ptr<Bus> bus,
    std::shared_ptr<Logger> logger
) {
    if (!this->cacheSet.contains(tag)) return;

    std::shared_ptr<CacheLineNode> node = this->cacheSet[tag];

    switch (node->state) {
        case CacheLineState::SHARED:
        case CacheLineState::EXCLUSIVE:
        case CacheLineState::MODIFIED:
            node->state = CacheLineState::INVALID;
            this->invalidate(threadID, tag, bus, logger);
            break;
        case CacheLineState::INVALID:
            throw std::logic_error("Invalid cache state for BusRdX in MESI");
        default:
            throw std::logic_error("Invalid cache state for MESI");
    }
}

void MESICacheSet::handleBusUpdateEvent(
    uint32_t tag,
    std::shared_ptr<Logger> logger
) {
    throw std::logic_error("Invalid bus event for MESI");
}

