#pragma once
#include <cstdint>
#include <vector>
#include <string>

struct PredictorStats {
    uint64_t total     = 0;
    uint64_t correct   = 0;
    uint64_t incorrect = 0;
    double accuracy() const {
        return total ? (double)correct / total * 100.0 : 0.0;
    }
};

class GSharePredictor {
public:
    // history_bits: number of bits in global history register
    // table_size:   number of entries (must be power of 2)
    explicit GSharePredictor(size_t history_bits = 10,
                             size_t table_size   = 1024);

    bool predict_and_update(uint64_t address, bool actual_taken);

    const PredictorStats& stats() const { return stats_; }
    void reset_stats();
    void print_stats(const std::string& label) const;

private:
    size_t               history_bits_;
    uint64_t             global_history_;  // shift register
    uint64_t             history_mask_;
    std::vector<uint8_t> table_;           // 2-bit saturating counters
    size_t               table_mask_;
    PredictorStats       stats_;

    size_t get_index(uint64_t address) const;
};

