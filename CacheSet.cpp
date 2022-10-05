#include "CacheSet.h"

#include <memory>

const int CACHE_HIT_COST = 1;
const int DIRTY_CACHE_EVICT_COST = 100;
const int MEMORY_FETCH_COST = 100;

Node::Node(uint32_t tag): tag{tag}, isDirty{false} {}

CacheSet::CacheSet(int associativity)
        : cacheSet{}
        , associativity{associativity}
        , size{0}
        , firstDummy{std::make_shared<Node>(0)}
        , lastDummy{std::make_shared<Node>(0)} {
    this->firstDummy->next = lastDummy;
    this->lastDummy->prev = firstDummy;
}

int CacheSet::read(uint32_t tag) {
    int numCycles = 0;
    std::shared_ptr<Node> node;

    auto iter = this->cacheSet.find(tag);
    if (iter != this->cacheSet.end()) {
        node = iter->second;
        this->removeNode(node);

        numCycles += CACHE_HIT_COST;
    } else {
        numCycles += MEMORY_FETCH_COST;
        node = std::make_shared<Node>(tag);
        cacheSet[tag] = node;

        if (this->size < this->associativity) {
            ++this->size;
        } else {
            numCycles += this->evict();
        }
    }

    this->insertNode(node);
    return numCycles;
}

int CacheSet::write(uint32_t tag) {
    int numCycles = this->read(tag);
    this->cacheSet[tag]->isDirty = true;

    return numCycles;
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
    std::shared_ptr<Node> last = this->lastDummy->prev;

    this->removeNode(last);
    this->cacheSet.erase(last->tag);

    return last->isDirty ? DIRTY_CACHE_EVICT_COST : 0;
}

void CacheSet::removeNode(std::shared_ptr<Node> node) {
    std::shared_ptr<Node> prev = node->prev, next = node->next;

    prev->next = next;
    next->prev = prev;

    node->prev = nullptr;
    node->next = nullptr;
}

void CacheSet::insertNode(std::shared_ptr<Node> node) {
    std::shared_ptr<Node> next = this->firstDummy->next;
    next->prev = node;

    node->next = next;
    node->prev = this->firstDummy;

    this->firstDummy->next = node;
}

