#include "MESIFCacheSet.hpp"

#include "Logger.hpp"

#include <memory>
#include <stdexcept>
#include <utility>

const int MEMORY_FETCH_COST = 100;

MESIFCacheSet::MESIFCacheSet(
    int setIdx,
    int numSetIdxBits,
    int associativity,
    int blockSize
) : CacheSet{setIdx, numSetIdxBits, associativity, blockSize} {}

MESIFCacheSet::~MESIFCacheSet() {};

int MESIFCacheSet::read(
    int threadID,
    uint32_t tag,
    std::shared_ptr<Bus> bus,
    std::shared_ptr<Logger> logger
) {
    int numCycles = CacheSet::read(threadID, tag, bus, logger);
    std::shared_ptr<CacheLineNode> node = this->cacheSet[tag];

    switch (node->state) {
        case CacheLineState::INVALID: {
            bool canFetchFromAnotherNode = bus->canFetchFromAnotherNode(this->getBlockIdx(tag));
            bool isExclusive = bus->busReadAndCheckIsExclusive(
                this->getBlockIdx(tag),
                threadID
            );

            if (isExclusive) {
                node->state = CacheLineState::EXCLUSIVE;
                numCycles += MEMORY_FETCH_COST;
                logger->addExecutionCycles(MEMORY_FETCH_COST);
                logger->addIdleCycles(MEMORY_FETCH_COST);
                bus->incrementBlockIdxShareableCount(this->getBlockIdx(tag));
            } else {

                if (canFetchFromAnotherNode) {
                    // A MESIF optimisation is to always put the most recent requester in FORWARD state
                    node->state = CacheLineState::FORWARD;
                    bus->incrementBlockIdxShareableCount(this->getBlockIdx(tag));
                    numCycles += this->numCyclesToSendBlock;
                    logger->incrementBusTraffic();
                    logger->addExecutionCycles(this->numCyclesToSendBlock);
                    logger->addIdleCycles(this->numCyclesToSendBlock);
                } else {
                    // If a read request is satisfied from main memory, the node enters SHARED state
                    node->state = CacheLineState::SHARED;
                    numCycles += MEMORY_FETCH_COST;
                    logger->addExecutionCycles(MEMORY_FETCH_COST);
                    logger->addIdleCycles(MEMORY_FETCH_COST);
                }
            }
        }
        case CacheLineState::SHARED:
        case CacheLineState::EXCLUSIVE:
        case CacheLineState::MODIFIED:
        case CacheLineState::FORWARD:
            break;
        default:
            throw std::logic_error("Invalid cache state for MESIF");
    }

    if (node->state == CacheLineState::EXCLUSIVE
            || node->state == CacheLineState::MODIFIED) {
        logger->incrementPrivateDataAccess();
    } else {
        logger->incrementPublicDataAccess();
    }

    return numCycles;
}

int MESIFCacheSet::write(
    int threadID,
    uint32_t tag,
    std::shared_ptr<Bus> bus,
    std::shared_ptr<Logger> logger
) {
    int numCycles = CacheSet::write(threadID, tag, bus, logger);
    std::shared_ptr<CacheLineNode> node = this->cacheSet[tag];

    switch (node->state) {
        case CacheLineState::INVALID: {
            bool canFetchFromAnotherNode = bus->canFetchFromAnotherNode(this->getBlockIdx(tag));
            bool isExclusive = bus->busReadExclusiveAndCheckIsExclusive(
                this->getBlockIdx(tag),
                threadID
            );

            if (isExclusive) {
                numCycles += MEMORY_FETCH_COST;

                logger->addExecutionCycles(MEMORY_FETCH_COST);
                logger->addIdleCycles(MEMORY_FETCH_COST);
            } else {
                if (canFetchFromAnotherNode) {
                    numCycles += this->numCyclesToSendBlock;
                    logger->incrementBusTraffic();
                    logger->addExecutionCycles(this->numCyclesToSendBlock);
                    logger->addIdleCycles(this->numCyclesToSendBlock);
                } else {
                    numCycles += MEMORY_FETCH_COST;
                    logger->addExecutionCycles(MEMORY_FETCH_COST);
                    logger->addIdleCycles(MEMORY_FETCH_COST);
                }
            }
            bus->incrementBlockIdxShareableCount(this->getBlockIdx(tag));
            break;
        }
        case CacheLineState::FORWARD:
            bus->decrementBlockIdxShareableCount(this->getBlockIdx(tag));
        case CacheLineState::SHARED:
            bus->busReadExclusiveAndCheckIsExclusive(
                this->getBlockIdx(tag),
                threadID
            );
            bus->incrementBlockIdxShareableCount(this->getBlockIdx(tag));
            break;
        case CacheLineState::EXCLUSIVE:
        case CacheLineState::MODIFIED:
            break;
        default:
            throw std::logic_error("Invalid cache state for MESIF");
    }

    logger->incrementPrivateDataAccess();
    node->state = CacheLineState::MODIFIED;
    return numCycles;
}

void MESIFCacheSet::handleBusReadEvent(
    uint32_t tag,
    std::shared_ptr<Bus> bus,
    std::shared_ptr<Logger> logger
) {
    if (!this->cacheSet.contains(tag)) return;

    std::shared_ptr<CacheLineNode> node = this->cacheSet[tag];
    
    switch (node->state) {
        case CacheLineState::FORWARD:
        case CacheLineState::EXCLUSIVE:
        case CacheLineState::MODIFIED:
            logger->incrementBusTraffic();
            logger->addExecutionCycles(this->numCyclesToSendBlock);
            logger->addIdleCycles(this->numCyclesToSendBlock);
            bus->decrementBlockIdxShareableCount(this->getBlockIdx(tag));
            node->state = CacheLineState::SHARED;
        // In MESIF, cache-to-cache sharing never happens from SHARED state.
        case CacheLineState::SHARED:
            break;
        case CacheLineState::INVALID:
            throw std::logic_error("Invalid cache state for BusRd in MESIF");
        default:
            throw std::logic_error("Invalid cache state for MESIF");
    }
}

void MESIFCacheSet::handleBusReadExclusiveEvent(
    int threadID,
    uint32_t tag,
    std::shared_ptr<Bus> bus,
    std::shared_ptr<Logger> logger
) {
    if (!this->cacheSet.contains(tag)) return;

    std::shared_ptr<CacheLineNode> node = this->cacheSet[tag];

    switch (node->state) {
        case CacheLineState::FORWARD:
        case CacheLineState::EXCLUSIVE:
        case CacheLineState::MODIFIED:
            logger->incrementBusTraffic();
            logger->addExecutionCycles(this->numCyclesToSendBlock);
            logger->addIdleCycles(this->numCyclesToSendBlock);
            bus->decrementBlockIdxShareableCount(this->getBlockIdx(tag));
        case CacheLineState::SHARED:
            node->state = CacheLineState::INVALID;
            this->invalidate(threadID, tag, bus, logger);
            break;
        case CacheLineState::INVALID:
            throw std::logic_error("Invalid cache state for BusRdX in MESIF");
        default:
            throw std::logic_error("Invalid cache state for MESIF");
    }
}

void MESIFCacheSet::handleBusUpdateEvent(
    uint32_t tag,
    std::shared_ptr<Logger> logger
) {
    throw std::logic_error("Invalid bus event for MESIF");
}