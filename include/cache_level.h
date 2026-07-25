#pragma once
#include "cache_block.h"
#include "policy.h"
#include <vector>
#include <cstdint>
#include <string>

struct CacheStats {
    uint64_t hits        = 0;
    uint64_t misses      = 0;
    uint64_t evictions   = 0;
    uint64_t writebacks  = 0;

    double hit_rate()  const { return (hits + misses) ? (double)hits  / (hits + misses) : 0.0; }
    double miss_rate() const { return (hits + misses) ? (double)misses / (hits + misses) : 0.0; }
};

class CacheLevel {
public:
    // size_bytes   : total cache size in bytes
    // associativity: number of ways (1 = direct-mapped)
    // block_size   : bytes per cache line
    CacheLevel(const std::string& name,
               size_t size_bytes,
               size_t associativity,
               size_t block_size,
               ReplacementPolicy policy  = ReplacementPolicy::LRU,
               WritePolicy       wpolicy = WritePolicy::WRITE_BACK,
               uint32_t          hit_latency_cycles = 4);

    // Returns true on hit, false on miss.
    // is_write=true for store instructions.
    bool access(uint64_t address, bool is_write);

    const CacheStats& stats() const { return stats_; }
    void              print_config() const;
    void              print_stats()  const;
    void              reset_stats();

private:
    std::string       name_;
    size_t            num_sets_;
    size_t            associativity_;
    size_t            block_size_;
    ReplacementPolicy policy_;
    WritePolicy       wpolicy_;
    uint32_t          hit_latency_;

    // sets_[set_index] holds `associativity_` blocks
    std::vector<std::vector<CacheBlock>> sets_;
    uint64_t timer_ = 0;   // logical clock for LRU/FIFO

    CacheStats stats_;

    // address decomposition
    uint64_t get_tag      (uint64_t addr) const;
    size_t   get_set_index(uint64_t addr) const;

    // find victim way index inside a set
    size_t find_victim(size_t set_idx);
};
