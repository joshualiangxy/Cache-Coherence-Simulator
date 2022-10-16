#include "DragonCacheSet.hpp"

#include "Logger.hpp"

#include <stdexcept>
#include <utility>

const int MEMORY_FETCH_COST = 100;

DragonCacheSet::DragonCacheSet(
    int setIdx,
    int numSetIdxBits,
    int associativity,
    int blockSize
) : CacheSet{setIdx, numSetIdxBits, associativity, blockSize} {}

DragonCacheSet::~DragonCacheSet() {};

int DragonCacheSet::read(
    int threadID,
    uint32_t tag,
    std::shared_ptr<Bus> bus,
    std::shared_ptr<Logger> logger
) {
    int numCycles = CacheSet::read(threadID, tag, bus, logger);
    std::shared_ptr<CacheLineNode> node = this->cacheSet[tag];

    switch (node->state) {
        case CacheLineState::INVALID: {
            bool isExclusive = bus->busReadAndCheckIsExclusive(
                this->getBlockIdx(tag),
                threadID
            );

            numCycles += MEMORY_FETCH_COST;
            logger->addExecutionCycles(MEMORY_FETCH_COST);
            logger->addIdleCycles(MEMORY_FETCH_COST);

            node->state = isExclusive
                ? CacheLineState::EXCLUSIVE
                : CacheLineState::SHARED_CLEAN;
            break;
        }
        case CacheLineState::EXCLUSIVE:
        case CacheLineState::MODIFIED:
        case CacheLineState::SHARED_CLEAN:
        case CacheLineState::SHARED_MODIFIED:
            break;
        default:
            throw std::logic_error("Invalid cache state for Dragon");
    }

    if (node->state == CacheLineState::EXCLUSIVE
            || node->state == CacheLineState::MODIFIED) {
        logger->incrementPrivateDataAccess();
    } else {
        logger->incrementPublicDataAccess();
    }

    return numCycles;
}

int DragonCacheSet::write(
    int threadID,
    uint32_t tag,
    std::shared_ptr<Bus> bus,
    std::shared_ptr<Logger> logger
) {
    int numCycles = CacheSet::write(threadID, tag, bus, logger);
    std::shared_ptr<CacheLineNode> node = this->cacheSet[tag];

    switch (node->state) {
        case CacheLineState::INVALID: {
            uint32_t blockIdx = this->getBlockIdx(tag);
            bool isExclusive = bus->busReadAndCheckIsExclusive(
                blockIdx,
                threadID
            );

            numCycles += MEMORY_FETCH_COST;
            logger->addExecutionCycles(MEMORY_FETCH_COST);
            logger->addIdleCycles(MEMORY_FETCH_COST);

            if (isExclusive) {
                node->state = CacheLineState::MODIFIED;
            } else {
                node->state = CacheLineState::SHARED_MODIFIED;

                bus->busUpdateAndCheckIsExclusive(blockIdx, threadID);
                logger->incrementBusInvalidateUpdateEvents();
                logger->incrementBusTraffic();
                logger->addExecutionCycles(this->numCyclesToSendBlock);
                logger->addIdleCycles(this->numCyclesToSendBlock);
            }

            break;
        }
        case CacheLineState::EXCLUSIVE:
        case CacheLineState::MODIFIED:
            node->state = CacheLineState::MODIFIED;
            break;
        case CacheLineState::SHARED_CLEAN:
        case CacheLineState::SHARED_MODIFIED: {
            bool isExclusive = bus->busUpdateAndCheckIsExclusive(
                this->getBlockIdx(tag),
                threadID
            );
            node->state = isExclusive
                ? CacheLineState::MODIFIED
                : CacheLineState::SHARED_MODIFIED;

            logger->incrementBusInvalidateUpdateEvents();
            logger->incrementBusTraffic();
            logger->addExecutionCycles(this->numCyclesToSendBlock);
            logger->addIdleCycles(this->numCyclesToSendBlock);
            break;
        }
        default:
            throw std::logic_error("Invalid cache state for Dragon");
    }

    return numCycles;
}

void DragonCacheSet::handleBusReadEvent(
    uint32_t tag,
    std::shared_ptr<Logger> logger
) {
    if (!this->cacheSet.contains(tag)) return;

    std::shared_ptr<CacheLineNode> node = this->cacheSet[tag];

    switch (node->state) {
        case CacheLineState::EXCLUSIVE:
            node->state = CacheLineState::SHARED_CLEAN;
            break;
        case CacheLineState::MODIFIED:
            node->state = CacheLineState::SHARED_MODIFIED;
            break;
        case CacheLineState::SHARED_CLEAN:
        case CacheLineState::SHARED_MODIFIED:
            break;
        case CacheLineState::INVALID:
            throw std::logic_error("Invalid cache state for BusRd in Dragon");
        default:
            throw std::logic_error("Invalid cache state for Dragon");
    }
}

void DragonCacheSet::handleBusReadExclusiveEvent(
    int threadID,
    uint32_t tag,
    std::shared_ptr<Bus> bus,
    std::shared_ptr<Logger> logger
) {
    throw std::logic_error("Invalid bus event for Dragon");
}

void DragonCacheSet::handleBusUpdateEvent(
    uint32_t tag,
    std::shared_ptr<Logger> logger
) {
    if (!this->cacheSet.contains(tag)) return;

    std::shared_ptr<CacheLineNode> node = this->cacheSet[tag];

    switch (node->state) {
        case CacheLineState::SHARED_CLEAN:
        case CacheLineState::SHARED_MODIFIED:
            node->state = CacheLineState::SHARED_CLEAN;
            break;
        case CacheLineState::INVALID:
        case CacheLineState::EXCLUSIVE:
        case CacheLineState::MODIFIED:
            throw std::logic_error("Invalid cache state for BusUpdate in Dragon");
        default:
            throw std::logic_error("Invalid cache state for Dragon");
    }

    logger->incrementBusTraffic();
    logger->addExecutionCycles(this->numCyclesToSendBlock);
    logger->addIdleCycles(this->numCyclesToSendBlock);
}

