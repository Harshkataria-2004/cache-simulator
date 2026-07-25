#pragma once
#include <cstdint>
#include <vector>
#include <string>

// 2-bit saturating counter states
// 00=Strong Not Taken, 01=Weak Not Taken
// 10=Weak Taken,       11=Strong Taken
enum class PredictorState : uint8_t {
    STRONG_NOT_TAKEN = 0,
    WEAK_NOT_TAKEN   = 1,
    WEAK_TAKEN       = 2,
    STRONG_TAKEN     = 3
};

struct BranchStats {
    uint64_t total      = 0;
    uint64_t correct    = 0;
    uint64_t incorrect  = 0;
    double accuracy() const {
        return total ? (double)correct / total * 100.0 : 0.0;
    }
};

class BranchPredictor {
public:
    // table_size: number of entries (must be power of 2)
    explicit BranchPredictor(size_t table_size = 1024);

    // address: branch PC, actual_taken: true if branch was taken
    // returns true if prediction was correct
    bool predict_and_update(uint64_t address, bool actual_taken);

    const BranchStats& stats() const { return stats_; }
    void reset_stats();
    void print_stats(const std::string& label) const;

private:
    std::vector<PredictorState> table_;
    size_t                      mask_;
    BranchStats                 stats_;

    size_t get_index(uint64_t address) const;
    bool   is_taken(PredictorState s) const { return s >= PredictorState::WEAK_TAKEN; }
    PredictorState increment(PredictorState s) const;
    PredictorState decrement(PredictorState s) const;
};

