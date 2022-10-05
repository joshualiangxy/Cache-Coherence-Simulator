#include "CacheLine.h"

#include <memory>

const int CACHE_HIT_COST = 1;
const int DIRTY_CACHE_EVICT_COST = 100;
const int MEMORY_FETCH_COST = 100;

Node::Node(uint32_t tag): tag{tag}, isDirty{false} {}

CacheLine::CacheLine(int associativity)
        : associativity{associativity}, size{0} {
    this->firstDummy = std::make_shared<Node>(0);
    this->lastDummy = std::make_shared<Node>(0);

    this->firstDummy->next = lastDummy;
    this->lastDummy->prev = firstDummy;
}

int CacheLine::read(uint32_t tag) {
    int numCycles = 0;
    std::shared_ptr<Node> node;

    auto iter = this->line.find(tag);
    if (iter != this->line.end()) {
        node = iter->second;
        this->removeNode(node);

        numCycles += CACHE_HIT_COST;
    } else {
        numCycles += MEMORY_FETCH_COST;
        node = std::make_shared<Node>(tag);
        line[tag] = node;

        if (this->size < this->associativity) {
            ++this->size;
        } else {
            numCycles += this->evict();
        }
    }

    this->insertNode(node);

    return numCycles;
}

int CacheLine::write(uint32_t tag) {
    int numCycles = this->read(tag);
    this->line[tag]->isDirty = true;

    return numCycles;
}

void CacheLine::invalidate(uint32_t tag) {
    auto iter = this->line.find(tag);
    if (iter == this->line.end()) {
        return;
    }

    --this->size;
    this->removeNode(iter->second);
    this->line.erase(tag);
}

int CacheLine::evict() {
    std::shared_ptr<Node> last = this->lastDummy->prev;

    this->removeNode(last);
    this->line.erase(last->tag);

    return last->isDirty ? DIRTY_CACHE_EVICT_COST : 0;
}

void CacheLine::removeNode(std::shared_ptr<Node> node) {
    std::shared_ptr<Node> prev = node->prev, next = node->next;

    prev->next = next;
    next->prev = prev;

    node->prev = nullptr;
    node->next = nullptr;
}

void CacheLine::insertNode(std::shared_ptr<Node> node) {
    std::shared_ptr<Node> next = this->firstDummy->next;
    next->prev = node;

    node->next = next;
    node->prev = this->firstDummy;

    this->firstDummy->next = node;
}

