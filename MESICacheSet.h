#ifndef GUARD_MESI_CACHE_SET_H
#define GUARD_MESI_CACHE_SET_H

#include "CacheSet.h"

#include <cstdint>
#include <utility>

class MESICacheSet : public CacheSet {
public:
    MESICacheSet(int associativity);
    virtual ~MESICacheSet();

    std::pair<bool, int> read(uint32_t tag);
    std::pair<bool, int> write(uint32_t tag);
};

#endif

