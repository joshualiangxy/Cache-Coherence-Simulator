#include "CacheSet.hpp"

#include "Logger.hpp"

#include <memory>
#include <utility>

const int CACHE_HIT_COST = 1;
const int DIRTY_CACHE_EVICT_COST = 100;

const int NUM_BYTES_IN_WORD = 4;
const int NUM_CYCLES_PER_WORD = 2;

CacheLineNode::CacheLineNode(uint32_t tag)
    : tag{tag}
    , isDirty{false}
    , state{CacheLineState::INVALID} {}

CacheSet::CacheSet(
    int setIdx,
    int numSetIdxBits,
    int associativity,
    int blockSize
) : cacheSet{},
    setIdx{setIdx},
    numSetIdxBits{numSetIdxBits},
    associativity{associativity},
    size{0},
    firstDummy{std::make_shared<CacheLineNode>(0)},
    lastDummy{std::make_shared<CacheLineNode>(0)}
{
    int numWordsInBlock = blockSize / NUM_BYTES_IN_WORD;
    this->numCyclesToSendBlock = numWordsInBlock * NUM_CYCLES_PER_WORD;
    this->firstDummy->next = lastDummy;
    this->lastDummy->prev = firstDummy;
}

int CacheSet::read(
    int threadID,
    uint32_t tag,
    std::shared_ptr<Bus> bus,
    std::shared_ptr<Logger> logger
) {
    int numCycles = this->loadFromMemory(threadID, tag, bus, logger);
    return numCycles;
}

int CacheSet::write(
    int threadID,
    uint32_t tag,
    std::shared_ptr<Bus> bus,
    std::shared_ptr<Logger> logger
) {
    int numCycles = this->loadFromMemory(threadID, tag, bus, logger);
    this->cacheSet[tag]->isDirty = true;

    return numCycles;
}

void CacheSet::invalidate(
    int threadID,
    uint32_t tag,
    std::shared_ptr<Bus> bus,
    std::shared_ptr<Logger> logger
) {
    auto iter = this->cacheSet.find(tag);
    if (iter == this->cacheSet.end()) {
        return;
    }

    --this->size;
    this->removeNode(iter->second);
    this->cacheSet.erase(tag);

    logger->incrementBusInvalidateUpdateEvents();
    bus->invalidateBlock(this->getBlockIdx(tag), threadID);
}

uint32_t CacheSet::getBlockIdx(uint32_t tag) {
    return (tag << this->numSetIdxBits) | this->setIdx;
}

int CacheSet::evict(int threadID, std::shared_ptr<Bus> bus) {
    std::shared_ptr<CacheLineNode> last = this->lastDummy->prev;

    bool isCacheShareable = 
        last->state == CacheLineState::FORWARD 
        || last->state == CacheLineState::MODIFIED 
        || last->state == CacheLineState::EXCLUSIVE;

    if (isCacheShareable) {
        bus->decrementBlockIdxShareableCount(this->getBlockIdx(last->tag));
    }
    
    this->removeNode(last);
    this->cacheSet.erase(last->tag);
    bus->invalidateBlock(this->getBlockIdx(last->tag), threadID);

    return last->isDirty ? DIRTY_CACHE_EVICT_COST : 0;
}

void CacheSet::removeNode(std::shared_ptr<CacheLineNode> node) {
    std::shared_ptr<CacheLineNode> prev = node->prev, next = node->next;

    prev->next = next;
    next->prev = prev;

    node->prev = nullptr;
    node->next = nullptr;
}

void CacheSet::insertNode(std::shared_ptr<CacheLineNode> node) {
    std::shared_ptr<CacheLineNode> next = this->firstDummy->next;
    next->prev = node;

    node->next = next;
    node->prev = this->firstDummy;

    this->firstDummy->next = node;
}

int CacheSet::loadFromMemory(
    int threadID,
    uint32_t tag,
    std::shared_ptr<Bus> bus,
    std::shared_ptr<Logger> logger
) {
    int numCycles = 0;
    std::shared_ptr<CacheLineNode> node;

    auto iter = this->cacheSet.find(tag);
    if (iter != this->cacheSet.end()) {
        node = iter->second;
        this->removeNode(node);

        numCycles += CACHE_HIT_COST;
    } else {
        logger->incrementNumCacheMiss();
        node = std::make_shared<CacheLineNode>(tag);
        cacheSet[tag] = node;

        if (this->size < this->associativity) {
            ++this->size;
        } else {
            numCycles += this->evict(threadID, bus);
        }
    }

    this->insertNode(node);

    logger->addExecutionCycles(numCycles);
    logger->addIdleCycles(numCycles);

    return numCycles;
}

