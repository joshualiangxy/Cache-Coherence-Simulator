#ifndef GUARD_CACHE_LINE_H
#define GUARD_CACHE_LINE_H

#include <cstdint>
#include <memory>
#include <unordered_map>
#include <utility>

enum class CacheLineState {
    INVALID,
    SHARED,
    EXCLUSIVE,
    MODIFIED,
    SHARED_CLEAN,
    SHARED_MODIFIED
};

struct CacheLineNode {
    std::shared_ptr<CacheLineNode> next;
    std::shared_ptr<CacheLineNode> prev;
    uint32_t tag;
    bool isDirty;
    CacheLineState state;

    CacheLineNode(uint32_t tag);
};

class CacheSet {
public:
    CacheSet(int associativity);

    virtual std::pair<bool, int> read(uint32_t tag);
    virtual std::pair<bool, int> write(uint32_t tag);
    void invalidate(uint32_t tag);

protected:
    std::unordered_map<uint32_t, std::shared_ptr<CacheLineNode>> cacheSet;

private:
    int associativity;
    int size;
    std::shared_ptr<CacheLineNode> firstDummy;
    std::shared_ptr<CacheLineNode> lastDummy;

    int evict();
    void removeNode(std::shared_ptr<CacheLineNode> node);
    void insertNode(std::shared_ptr<CacheLineNode> node);
};

#endif

