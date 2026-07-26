
#include "cache_hierarchy.h"
#include "trace_reader.h"
#include <iostream>
#include <memory>
#include <string>
#include <cstdlib>

int main(int argc, char* argv[]) {
    if (argc < 6) {
        std::cerr << "Usage: cache_sweep <trace> <l1_kb> <l2_kb> <l3_kb> <assoc>\n";
        return 1;
    }
    srand(42);
    std::string trace = argv[1];
    size_t l1_kb  = std::stoul(argv[2]);
    size_t l2_kb  = std::stoul(argv[3]);
    size_t l3_kb  = std::stoul(argv[4]);
    size_t assoc  = std::stoul(argv[5]);

    auto h = std::make_unique<CacheHierarchy>();
    h->add_level(std::make_unique<CacheLevel>(
        "L1", l1_kb*1024, assoc, 64,
        ReplacementPolicy::LRU, WritePolicy::WRITE_BACK, 4));
    h->add_level(std::make_unique<CacheLevel>(
        "L2", l2_kb*1024, 8, 64,
        ReplacementPolicy::LRU, WritePolicy::WRITE_BACK, 12));
    h->add_level(std::make_unique<CacheLevel>(
        "L3", l3_kb*1024, 16, 64,
        ReplacementPolicy::LRU, WritePolicy::WRITE_BACK, 36));

    TraceReader reader(trace);
    MemAccess acc;
    while ((acc = reader.next()).valid)
        h->access(acc.address, acc.is_write);

    double amat = h->amat(200);
    std::cout << amat << "\n";
    return 0;
}
