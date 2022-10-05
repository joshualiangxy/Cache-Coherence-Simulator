#ifndef GUARD_CACHE_LINE_H
#define GUARD_CACHE_LINE_H

#include <cstdint>
#include <memory>
#include <unordered_map>

struct Node {
    std::shared_ptr<Node> next;
    std::shared_ptr<Node> prev;
    uint32_t tag;
    bool isDirty;

    Node(uint32_t tag);
};

class CacheSet {
public:
    CacheSet(int associativity);

    int read(uint32_t tag);
    int write(uint32_t tag);
    void invalidate(uint32_t tag);

private:
    std::unordered_map<uint32_t, std::shared_ptr<Node>> cacheSet;
    int associativity;
    int size;
    std::shared_ptr<Node> firstDummy;
    std::shared_ptr<Node> lastDummy;

    int evict();
    void removeNode(std::shared_ptr<Node> node);
    void insertNode(std::shared_ptr<Node> node);
};

#endif

