#include "Cache.hpp"
#include "CacheSet.hpp"
#include "DragonCacheSet.hpp"
#include "Logger.hpp"
#include "MESICacheSet.hpp"
#include "MESIFCacheSet.hpp"

#include <cmath>
#include <cstdint>
#include <memory>

const uint32_t NUM_ADDRESS_BITS = 32;

Cache::Cache(
    int threadID,
    int cacheSize,
    int associativity,
    int blockSize,
    CacheType cacheType,
    std::shared_ptr<Logger> logger
) : threadID{threadID}, associativity{associativity}, logger{std::move(logger)} {
    int numBlocks = cacheSize / blockSize;
    int numSets = numBlocks / associativity;

    this->numOffsetBits = std::log2(blockSize);
    this->numSetIdxBits = std::log2(numSets);
    this->numTagBits = NUM_ADDRESS_BITS - numOffsetBits - numSetIdxBits;

    this->blkOffsetMask = this->numOffsetBits == 0
        ? 0
        : 0xFFFFFFFF >> (NUM_ADDRESS_BITS - this->numOffsetBits);
    this->setIdxMask = this->numSetIdxBits == 0
        ? 0
        : 0xFFFFFFFF >> (NUM_ADDRESS_BITS - this->numSetIdxBits)
            << this->numOffsetBits;
    this->tagMask = this->numTagBits == 0
        ? 0
        : 0xFFFFFFFF << (NUM_ADDRESS_BITS - this->numTagBits);

    this->cacheSets.reserve(numSets);
    for (int setIdx = 0; setIdx < numSets; ++setIdx) {
        switch (cacheType) {
            case CacheType::MESI:
                this->cacheSets.emplace_back(
                    std::make_shared<MESICacheSet>(
                        setIdx,
                        this->numSetIdxBits,
                        associativity,
                        blockSize
                    )
                );
                break;

            case CacheType::DRAGON:
                this->cacheSets.emplace_back(
                    std::make_shared<DragonCacheSet>(
                        setIdx,
                        this->numSetIdxBits,
                        associativity,
                        blockSize
                    )
                );
                break;
            
            case CacheType::MESIF:
                this->cacheSets.emplace_back(
                    std::make_shared<MESIFCacheSet>(
                        setIdx,
                        this->numSetIdxBits,
                        associativity,
                        blockSize
                    )
                );
                break;
        }
    }
}

int Cache::read(uint32_t address, std::shared_ptr<Bus> bus) {
    uint32_t setIdx = this->getSetIdx(address), tag = this->getTag(address);
    int numCycles = this->cacheSets[setIdx]->read(
        this->threadID,
        tag,
        bus,
        this->logger
    );

    this->logger->incrementNumLoadStoreInstructions();

    return numCycles;
}

int Cache::write(uint32_t address, std::shared_ptr<Bus> bus) {
    uint32_t setIdx = this->getSetIdx(address), tag = this->getTag(address);
    int numCycles = this->cacheSets[setIdx]->write(
        this->threadID,
        tag,
        bus,
        this->logger
    );

    this->logger->incrementNumLoadStoreInstructions();

    return numCycles;
}

void Cache::handleBusEvents(
    std::queue<BusEvent>& eventQueue,
    std::shared_ptr<Bus> bus
) {
    while (!eventQueue.empty()) {
        BusEvent event = eventQueue.front();
        eventQueue.pop();

        switch (event.type) {
            case BusEventType::BUS_READ:
                this->handleBusReadEvent(event.blockIdx);
                break;
            case BusEventType::BUS_READ_EXCLUSIVE:
                this->handleBusReadExclusiveEvent(event.blockIdx, bus);
                break;
            case BusEventType::BUS_UPDATE:
                this->handleBusUpdateEvent(event.blockIdx);
                break;
        }
    }
}

uint32_t Cache::getSetIdx(uint32_t address) {
    return (address & this->setIdxMask) >> this->numOffsetBits;
}

uint32_t Cache::getTag(uint32_t address) {
    return (address & this->tagMask)
        >> (this->numOffsetBits + this->numSetIdxBits);
}

uint32_t Cache::getBlockIdx(uint32_t address) {
    return (address & (this->tagMask | this->setIdxMask))
        >> this->numOffsetBits;
}

void Cache::handleBusReadEvent(uint32_t blockIdx) {
    uint32_t address = blockIdx << this->numOffsetBits,
        setIdx = this->getSetIdx(address),
        tag = this->getTag(address);

    this->cacheSets[setIdx]->handleBusReadEvent(tag, this->logger);
}

void Cache::handleBusReadExclusiveEvent(
    uint32_t blockIdx,
    std::shared_ptr<Bus> bus
) {
    uint32_t address = blockIdx << this->numOffsetBits,
        setIdx = this->getSetIdx(address),
        tag = this->getTag(address);

    this->cacheSets[setIdx]->handleBusReadExclusiveEvent(
        this->threadID,
        tag,
        bus,
        this->logger
    );
}

void Cache::handleBusUpdateEvent(uint32_t blockIdx) {
    uint32_t address = blockIdx << this->numOffsetBits,
        setIdx = this->getSetIdx(address),
        tag = this->getTag(address);

    this->cacheSets[setIdx]->handleBusUpdateEvent(tag, this->logger);
}

