#ifndef GUARD_CACHE_H
#define GUARD_CACHE_H

enum class CacheType { MESI, DRAGON };

class Cache {
public:
    Cache();
    virtual ~Cache();
};

#endif

