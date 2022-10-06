#ifndef GUARD_MESI_CACHE_SET_H
#define GUARD_MESI_CACHE_SET_H

#include "CacheSet.h"
#include "Logger.h"

#include <cstdint>
#include <memory>
#include <utility>

class MESICacheSet : public CacheSet {
public:
    MESICacheSet(int associativity);
    virtual ~MESICacheSet();

    void read(uint32_t tag, std::shared_ptr<Logger> logger);
    void write(uint32_t tag, std::shared_ptr<Logger> logger);
};

#endif

