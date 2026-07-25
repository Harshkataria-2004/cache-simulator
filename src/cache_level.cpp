#include "cache_level.h"
#include <cmath>
#include <cassert>
#include <iostream>
#include <iomanip>
#include <climits>
#include <cstdlib>   // rand

// ── helpers ──────────────────────────────────────────────────────────────────

static size_t log2_exact(size_t v) {
    size_t r = 0;
    while (v >>= 1) ++r;
    return r;
}

// ── constructor ───────────────────────────────────────────────────────────────

CacheLevel::CacheLevel(const std::string& name,
                       size_t size_bytes,
                       size_t associativity,
                       size_t block_size,
                       ReplacementPolicy policy,
                       WritePolicy       wpolicy,
                       uint32_t          hit_latency_cycles)
    : name_(name),
      associativity_(associativity),
      block_size_(block_size),
      policy_(policy),
      wpolicy_(wpolicy),
      hit_latency_(hit_latency_cycles)
{
    assert(size_bytes > 0 && associativity > 0 && block_size > 0);
    assert((size_bytes % (associativity * block_size)) == 0 &&
           "Cache size must be divisible by (ways × block_size)");

    num_sets_ = size_bytes / (associativity * block_size);
    sets_.assign(num_sets_, std::vector<CacheBlock>(associativity_));
}

// ── address decomposition ─────────────────────────────────────────────────────

size_t CacheLevel::get_set_index(uint64_t addr) const {
    size_t offset_bits = log2_exact(block_size_);
    size_t index_bits  = log2_exact(num_sets_);
    return (addr >> offset_bits) & ((1ULL << index_bits) - 1);
}

uint64_t CacheLevel::get_tag(uint64_t addr) const {
    size_t offset_bits = log2_exact(block_size_);
    size_t index_bits  = log2_exact(num_sets_);
    return addr >> (offset_bits + index_bits);
}

// ── victim selection ──────────────────────────────────────────────────────────

size_t CacheLevel::find_victim(size_t set_idx) {
    auto& set = sets_[set_idx];

    // 1. prefer an invalid (empty) block
    for (size_t w = 0; w < associativity_; ++w)
        if (!set[w].valid) return w;

    // 2. apply replacement policy
    size_t victim = 0;
    switch (policy_) {

    case ReplacementPolicy::LRU: {
        uint64_t oldest = ULLONG_MAX;
        for (size_t w = 0; w < associativity_; ++w)
            if (set[w].last_used < oldest) { oldest = set[w].last_used; victim = w; }
        break;
    }
    case ReplacementPolicy::FIFO: {
        uint64_t oldest = ULLONG_MAX;
        for (size_t w = 0; w < associativity_; ++w)
            if (set[w].load_time < oldest) { oldest = set[w].load_time; victim = w; }
        break;
    }
    case ReplacementPolicy::LFU: {
        uint64_t min_freq = ULLONG_MAX;
        for (size_t w = 0; w < associativity_; ++w)
            if (set[w].freq < min_freq) { min_freq = set[w].freq; victim = w; }
        break;
    }
    case ReplacementPolicy::RANDOM:
        victim = rand() % associativity_;
        break;
    }
    return victim;
}

// ── main access function ──────────────────────────────────────────────────────

bool CacheLevel::access(uint64_t address, bool is_write) {
    ++timer_;
    size_t   set_idx = get_set_index(address);
    uint64_t tag     = get_tag(address);
    auto&    set     = sets_[set_idx];

    // --- HIT? ---
    for (size_t w = 0; w < associativity_; ++w) {
        if (set[w].valid && set[w].tag == tag) {
            ++stats_.hits;
            set[w].last_used = timer_;
            ++set[w].freq;
            if (is_write) {
                set[w].dirty = true;
                if (wpolicy_ == WritePolicy::WRITE_THROUGH)
                    ++stats_.writebacks; // propagate immediately
            }
            return true;  // HIT
        }
    }

    // --- MISS ---
    ++stats_.misses;
    size_t victim = find_victim(set_idx);

    // writeback if dirty
    if (set[victim].valid && set[victim].dirty) {
        ++stats_.evictions;
        ++stats_.writebacks;
    } else if (set[victim].valid) {
        ++stats_.evictions;
    }

    // install new block
    set[victim].tag       = tag;
    set[victim].valid     = true;
    set[victim].dirty     = is_write;
    set[victim].last_used = timer_;
    set[victim].load_time = timer_;
    set[victim].freq      = 1;

    return false;  // MISS
}

// ── diagnostics ───────────────────────────────────────────────────────────────

void CacheLevel::print_config() const {
    size_t total = num_sets_ * associativity_ * block_size_;
    std::cout << "[" << name_ << "] "
              << total / 1024 << " KB | "
              << num_sets_ << " sets × "
              << associativity_ << " ways × "
              << block_size_ << "B blocks | "
              << "lat=" << hit_latency_ << " cycles\n";
}

void CacheLevel::print_stats() const {
    uint64_t total = stats_.hits + stats_.misses;
    std::cout << "[" << name_ << "] "
              << "accesses=" << total
              << "  hits="       << stats_.hits
              << "  misses="     << stats_.misses
              << "  hit_rate="   << std::fixed << std::setprecision(2)
                                 << stats_.hit_rate() * 100 << "%"
              << "  evictions="  << stats_.evictions
              << "  writebacks=" << stats_.writebacks
              << "\n";
}

void CacheLevel::reset_stats() { stats_ = {}; timer_ = 0; }
