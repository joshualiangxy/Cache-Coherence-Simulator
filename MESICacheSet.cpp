#include "MESICacheSet.hpp"

#include "Logger.hpp"

#include <memory>
#include <stdexcept>
#include <utility>

const int MEMORY_FETCH_COST = 100;

MESICacheSet::MESICacheSet(
    int setIdx,
    int numSetIdxBits,
    int associativity,
    int blockSize
) : CacheSet{setIdx, numSetIdxBits, associativity, blockSize} {}

MESICacheSet::~MESICacheSet() {};

bool MESICacheSet::read(
    int threadID,
    uint32_t tag,
    std::shared_ptr<Bus> bus,
    std::shared_ptr<Logger> logger
) {
    bool isCacheMiss = CacheSet::read(threadID, tag, bus, logger);
    std::shared_ptr<CacheLineNode> node = this->cacheSet[tag];

    switch (node->state) {
        case CacheLineState::INVALID: {
            bool isExclusive = bus->busReadAndCheckIsExclusive(
                this->getBlockIdx(tag),
                threadID
            );

            if (isExclusive) {
                node->state = CacheLineState::EXCLUSIVE;

                logger->addExecutionCycles(MEMORY_FETCH_COST);
                logger->addIdleCycles(MEMORY_FETCH_COST);
            } else {
                node->state = CacheLineState::SHARED;

                logger->incrementBusTraffic();
                logger->addExecutionCycles(this->numCyclesToSendBlock);
                logger->addIdleCycles(this->numCyclesToSendBlock);
            }
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

    return isCacheMiss;
}

bool MESICacheSet::write(
    int threadID,
    uint32_t tag,
    std::shared_ptr<Bus> bus,
    std::shared_ptr<Logger> logger
) {
    bool isCacheMiss = CacheSet::write(threadID, tag, bus, logger);
    std::shared_ptr<CacheLineNode> node = this->cacheSet[tag];

    switch (node->state) {
        case CacheLineState::INVALID: {
            bool isExclusive = bus->busReadExclusiveAndCheckIsExclusive(
                this->getBlockIdx(tag),
                threadID
            );

            if (isExclusive) {
                logger->addExecutionCycles(MEMORY_FETCH_COST);
                logger->addIdleCycles(MEMORY_FETCH_COST);
            } else {
                logger->incrementBusTraffic();
                logger->addExecutionCycles(this->numCyclesToSendBlock);
                logger->addIdleCycles(this->numCyclesToSendBlock);
            }
            break;
        }
        case CacheLineState::SHARED:
            bus->busReadExclusiveAndCheckIsExclusive(
                this->getBlockIdx(tag),
                threadID
            );
            break;
        case CacheLineState::EXCLUSIVE:
        case CacheLineState::MODIFIED:
            break;
        default:
            throw std::logic_error("Invalid cache state for MESI");
    }

    node->state = CacheLineState::MODIFIED;
    return isCacheMiss;
}

void MESICacheSet::handleBusReadEvent(
    uint32_t tag,
    std::shared_ptr<Logger> logger
) {
    if (!this->cacheSet.contains(tag)) return;

    std::shared_ptr<CacheLineNode> node = this->cacheSet[tag];

    switch (node->state) {
        case CacheLineState::EXCLUSIVE:
        case CacheLineState::MODIFIED:
            logger->incrementBusTraffic();
            logger->addExecutionCycles(this->numCyclesToSendBlock);
            logger->addIdleCycles(this->numCyclesToSendBlock);
        case CacheLineState::SHARED:
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
        case CacheLineState::EXCLUSIVE:
        case CacheLineState::MODIFIED:
            logger->incrementBusTraffic();
            logger->addExecutionCycles(this->numCyclesToSendBlock);
            logger->addIdleCycles(this->numCyclesToSendBlock);
        case CacheLineState::SHARED:
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

