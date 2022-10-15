#ifndef GUARD_CACHE_LINE_H
#define GUARD_CACHE_LINE_H

#include "Bus.hpp"
#include "Logger.hpp"

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
    CacheSet(int setIdx, int numSetIdxBits, int associativity, int blockSize);

    virtual bool read(
        int threadID,
        uint32_t tag,
        std::shared_ptr<Bus> bus,
        std::shared_ptr<Logger> logger
    );
    virtual bool write(
        int threadID,
        uint32_t tag,
        std::shared_ptr<Bus> bus,
        std::shared_ptr<Logger> logger
    );
    void invalidate(
        int threadID,
        uint32_t tag,
        std::shared_ptr<Bus> bus,
        std::shared_ptr<Logger> logger
    );

    virtual void handleBusReadEvent(
        uint32_t tag,
        std::shared_ptr<Logger> logger
    ) = 0;
    virtual void handleBusReadExclusiveEvent(
        int threadID,
        uint32_t tag,
        std::shared_ptr<Bus> bus,
        std::shared_ptr<Logger> logger
    ) = 0;
    virtual void handleBusUpdateEvent(
        uint32_t tag,
        std::shared_ptr<Logger> logger
    ) = 0;

protected:
    std::unordered_map<uint32_t, std::shared_ptr<CacheLineNode>> cacheSet;
    int numCyclesToSendBlock;

    uint32_t getBlockIdx(uint32_t tag);

private:
    int setIdx;
    int numSetIdxBits;
    int associativity;
    int size;
    std::shared_ptr<CacheLineNode> firstDummy;
    std::shared_ptr<CacheLineNode> lastDummy;

    int evict(int threadID, std::shared_ptr<Bus> bus);
    void removeNode(std::shared_ptr<CacheLineNode> node);
    void insertNode(std::shared_ptr<CacheLineNode> node);

    bool loadFromMemory(
        int threadID,
        uint32_t tag,
        std::shared_ptr<Bus> bus,
        std::shared_ptr<Logger> logger
    );
};

#endif

