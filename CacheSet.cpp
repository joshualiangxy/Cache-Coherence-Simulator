#include "CacheSet.h"

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

std::pair<bool, int> CacheSet::read(uint32_t tag) {
    int numCycles = 0;
    std::shared_ptr<CacheLineNode> node;
    bool isCacheMiss = false;

    auto iter = this->cacheSet.find(tag);
    if (iter != this->cacheSet.end()) {
        node = iter->second;
        this->removeNode(node);

        numCycles += CACHE_HIT_COST;
    } else {
        isCacheMiss = true;
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
    return { isCacheMiss, numCycles };
}

std::pair<bool, int> CacheSet::write(uint32_t tag) {
    auto [isCacheMiss, numCycles] = this->read(tag);
    this->cacheSet[tag]->isDirty = true;

    return { isCacheMiss, numCycles };
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

