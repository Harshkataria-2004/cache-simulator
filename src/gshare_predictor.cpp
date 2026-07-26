#include "gshare_predictor.h"
#include <iostream>
#include <iomanip>

GSharePredictor::GSharePredictor(size_t history_bits, size_t table_size)
    : history_bits_(history_bits),
      global_history_(0),
      history_mask_((1ULL << history_bits) - 1),
      table_(table_size, 2),  // start weakly taken
      table_mask_(table_size - 1)
{}

size_t GSharePredictor::get_index(uint64_t address) const {
    uint64_t pc_bits = (address >> 2) & table_mask_;
    return (pc_bits ^ global_history_) & table_mask_;
}

bool GSharePredictor::predict_and_update(uint64_t address, bool actual_taken) {
    size_t idx       = get_index(address);
    bool   predicted = table_[idx] >= 2;  // taken if counter >= 2
    bool   correct   = (predicted == actual_taken);

    ++stats_.total;
    if (correct) ++stats_.correct;
    else         ++stats_.incorrect;

    // update 2-bit saturating counter
    if (actual_taken && table_[idx] < 3) ++table_[idx];
    if (!actual_taken && table_[idx] > 0) --table_[idx];

    // update global history register (shift left, insert new outcome)
    global_history_ = ((global_history_ << 1) | actual_taken) & history_mask_;

    return correct;
}

void GSharePredictor::reset_stats() {
    stats_         = {};
    global_history_ = 0;
}

void GSharePredictor::print_stats(const std::string& label) const {
    std::cout << "[GShare:" << label << "] "
              << "total="     << stats_.total
              << "  correct=" << stats_.correct
              << "  wrong="   << stats_.incorrect
              << "  accuracy=" << std::fixed << std::setprecision(2)
              << stats_.accuracy() << "%\n";
}

