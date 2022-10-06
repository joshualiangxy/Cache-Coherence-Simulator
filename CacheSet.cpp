#include "CacheSet.h"
#include "Logger.h"

#include <memory>
#include <utility>

const int CACHE_HIT_COST = 1;
const int DIRTY_CACHE_EVICT_COST = 100;
const int MEMORY_FETCH_COST = 100;

CacheLineNode::CacheLineNode(uint32_t tag)
    : tag{tag}
    , isDirty{false}
    , state{CacheLineState::INVALID} {}

CacheSet::CacheSet(int associativity)
        : cacheSet{}
        , associativity{associativity}
        , size{0}
        , firstDummy{std::make_shared<CacheLineNode>(0)}
        , lastDummy{std::make_shared<CacheLineNode>(0)} {
    this->firstDummy->next = lastDummy;
    this->lastDummy->prev = firstDummy;
}

void CacheSet::read(uint32_t tag, std::shared_ptr<Logger> logger) {
    this->loadFromMemory(tag, logger);
}

void CacheSet::write(uint32_t tag, std::shared_ptr<Logger> logger) {
    this->loadFromMemory(tag, logger);
    this->cacheSet[tag]->isDirty = true;
}

void CacheSet::invalidate(uint32_t tag) {
    auto iter = this->cacheSet.find(tag);
    if (iter == this->cacheSet.end()) {
        return;
    }

    --this->size;
    this->removeNode(iter->second);
    this->cacheSet.erase(tag);
}

int CacheSet::evict() {
    std::shared_ptr<CacheLineNode> last = this->lastDummy->prev;

    this->removeNode(last);
    this->cacheSet.erase(last->tag);

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

void CacheSet::loadFromMemory(
    uint32_t tag,
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
        numCycles += MEMORY_FETCH_COST;
        node = std::make_shared<CacheLineNode>(tag);
        cacheSet[tag] = node;

        if (this->size < this->associativity) {
            ++this->size;
        } else {
            numCycles += this->evict();
        }
    }

    this->insertNode(node);

    logger->addExecutionCycles(numCycles);
    logger->addIdleCycles(numCycles);
}

