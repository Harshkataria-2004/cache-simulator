#pragma once
#include <cstdint>
#include <cstddef>

struct CacheBlock {
    uint64_t tag      = 0;
    bool     valid    = false;
    bool     dirty    = false;
    uint64_t last_used = 0;   // for LRU
    uint64_t load_time = 0;   // for FIFO
    uint64_t freq      = 0;   // for LFU
};
