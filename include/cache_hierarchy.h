#pragma once
#include "cache_level.h"
#include <vector>
#include <memory>
#include <string>

// Simulates a multi-level cache hierarchy (L1 → L2 → L3 → DRAM).
// On a miss at level N the request is forwarded to level N+1.
class CacheHierarchy {
public:
    // Add levels in order: L1 first, then L2, then L3.
    void add_level(std::unique_ptr<CacheLevel> level);

    // Process a single memory access.
    // Returns the level index that served the request (0=L1 hit, 1=L2 hit, …,
    // levels.size()=DRAM).
    int access(uint64_t address, bool is_write);

    // Compute AMAT given DRAM latency in cycles.
    double amat(uint32_t dram_latency_cycles) const;

    void print_config() const;
    void print_stats()  const;
    void reset_stats();

    // Per-level hit latencies (set when you add a level).
    const std::vector<uint32_t>& hit_latencies() const { return hit_latencies_; }

private:
    std::vector<std::unique_ptr<CacheLevel>> levels_;
    std::vector<uint32_t>                    hit_latencies_;
    uint64_t total_accesses_ = 0;
};
