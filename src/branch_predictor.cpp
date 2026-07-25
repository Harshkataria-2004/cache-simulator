#include "branch_predictor.h"
#include <iostream>
#include <iomanip>

BranchPredictor::BranchPredictor(size_t table_size)
    : table_(table_size, PredictorState::WEAK_TAKEN),
      mask_(table_size - 1)
{}

size_t BranchPredictor::get_index(uint64_t address) const {
    return (address >> 2) & mask_;
}

PredictorState BranchPredictor::increment(PredictorState s) const {
    return s == PredictorState::STRONG_TAKEN ? s :
           static_cast<PredictorState>(static_cast<uint8_t>(s) + 1);
}

PredictorState BranchPredictor::decrement(PredictorState s) const {
    return s == PredictorState::STRONG_NOT_TAKEN ? s :
           static_cast<PredictorState>(static_cast<uint8_t>(s) - 1);
}

bool BranchPredictor::predict_and_update(uint64_t address, bool actual_taken) {
    size_t idx         = get_index(address);
    PredictorState cur = table_[idx];
    bool predicted     = is_taken(cur);
    bool correct       = (predicted == actual_taken);

    ++stats_.total;
    if (correct) ++stats_.correct;
    else         ++stats_.incorrect;

    // update state
    table_[idx] = actual_taken ? increment(cur) : decrement(cur);
    return correct;
}

void BranchPredictor::reset_stats() { stats_ = {}; }

void BranchPredictor::print_stats(const std::string& label) const {
    std::cout << "[Branch:" << label << "] "
              << "total="    << stats_.total
              << "  correct=" << stats_.correct
              << "  wrong="   << stats_.incorrect
              << "  accuracy=" << std::fixed << std::setprecision(2)
              << stats_.accuracy() << "%\n";
}

