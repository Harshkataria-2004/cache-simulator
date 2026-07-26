#include "cache_hierarchy.h"
#include <iostream>
#include <iomanip>
#include <stdexcept>

void CacheHierarchy::add_level(std::unique_ptr<CacheLevel> level) {
    // We store hit latency separately (already set inside CacheLevel, but we
    // need it here for AMAT).  For now accept a default via a small helper.
    hit_latencies_.push_back(0); // placeholder – overridden in AMAT calc
    levels_.push_back(std::move(level));
}

int CacheHierarchy::access(uint64_t address, bool is_write) {
    ++total_accesses_;
    for (int i = 0; i < (int)levels_.size(); ++i) {
        if (levels_[i]->access(address, is_write))
            return i;   // served at this level
    }
    return (int)levels_.size();  // DRAM
}

// AMAT = H1 + M1*(H2 + M2*(H3 + M3*t_dram))
// where Hx = hit latency of level x, Mx = miss rate of level x
double CacheHierarchy::amat(uint32_t dram_latency_cycles) const {
    if (levels_.empty()) return dram_latency_cycles;

    // Walk from innermost to outermost
    double result = dram_latency_cycles;
    for (int i = (int)levels_.size() - 1; i >= 0; --i) {
        const auto& s = levels_[i]->stats();
        double mr = s.miss_rate();
        // We store hit latency inside CacheLevel but don't expose it as a
        // getter yet – use sensible defaults: L1=4, L2=12, L3=36 cycles
        uint32_t hl = levels_[i]->hit_latency();
        result = hl + mr * result;
    }
    return result;
}

void CacheHierarchy::print_config() const {
    std::cout << "=== Cache Hierarchy Configuration ===\n";
    for (auto& l : levels_) l->print_config();
    std::cout << "=====================================\n";
}

void CacheHierarchy::print_stats() const {
    std::cout << "\n=== Cache Statistics ===\n";
    for (auto& l : levels_) l->print_stats();
    std::cout << "========================\n";
}

void CacheHierarchy::reset_stats() {
    total_accesses_ = 0;
    for (auto& l : levels_) l->reset_stats();
}
