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

int MESICacheSet::read(
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

            if (isExclusive) {
                node->state = CacheLineState::EXCLUSIVE;

                numCycles += MEMORY_FETCH_COST;
                logger->addExecutionCycles(MEMORY_FETCH_COST);
                logger->addIdleCycles(MEMORY_FETCH_COST);
            } else {
                node->state = CacheLineState::SHARED;

                numCycles += this->numCyclesToSendBlock;
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

    return numCycles;
}

int MESICacheSet::write(
    int threadID,
    uint32_t tag,
    std::shared_ptr<Bus> bus,
    std::shared_ptr<Logger> logger
) {
    int numCycles = CacheSet::write(threadID, tag, bus, logger);
    std::shared_ptr<CacheLineNode> node = this->cacheSet[tag];

    switch (node->state) {
        case CacheLineState::INVALID: {
            bool isExclusive = bus->busReadExclusiveAndCheckIsExclusive(
                this->getBlockIdx(tag),
                threadID
            );

            if (isExclusive) {
                numCycles += MEMORY_FETCH_COST;

                logger->addExecutionCycles(MEMORY_FETCH_COST);
                logger->addIdleCycles(MEMORY_FETCH_COST);
            } else {
                numCycles += this->numCyclesToSendBlock;

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

    logger->incrementPrivateDataAccess();
    node->state = CacheLineState::MODIFIED;
    return numCycles;
}

void MESICacheSet::handleBusReadEvent(
    uint32_t tag,
    std::shared_ptr<Logger> logger
) {
    if (!this->cacheSet.contains(tag)) return;

    std::shared_ptr<CacheLineNode> node = this->cacheSet[tag];

    switch (node->state) {
        case CacheLineState::MODIFIED:
            logger->addExecutionCycles(MEMORY_FETCH_COST);
            logger->addIdleCycles(MEMORY_FETCH_COST);
        case CacheLineState::EXCLUSIVE:
        case CacheLineState::SHARED:
            logger->incrementBusTraffic();
            logger->addExecutionCycles(this->numCyclesToSendBlock);
            logger->addIdleCycles(this->numCyclesToSendBlock);

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
        case CacheLineState::MODIFIED:
            logger->addExecutionCycles(MEMORY_FETCH_COST);
            logger->addIdleCycles(MEMORY_FETCH_COST);
        case CacheLineState::EXCLUSIVE:
        case CacheLineState::SHARED:
            logger->incrementBusTraffic();
            logger->addExecutionCycles(this->numCyclesToSendBlock);
            logger->addIdleCycles(this->numCyclesToSendBlock);

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

