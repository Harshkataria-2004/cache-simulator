#include "cache_hierarchy.h"
#include "trace_reader.h"
#include <iostream>
#include <memory>
#include <string>
#include <vector>
#include <cstdlib>

// ── tiny synthetic workload (matrix-like sequential + strided accesses) ───────

static std::vector<uint64_t> generate_matrix_trace(int N = 64) {
    // Simulates accessing a row-major N×N matrix of 4-byte ints
    std::vector<uint64_t> addrs;
    uint64_t base = 0x10000;
    int stride    = N * 4;

    // Row-major order (cache-friendly)
    for (int i = 0; i < N; ++i)
        for (int j = 0; j < N; ++j)
            addrs.push_back(base + (uint64_t)(i * N + j) * 4);

    // Column-major order (cache-unfriendly, strided)
    for (int j = 0; j < N; ++j)
        for (int i = 0; i < N; ++i)
            addrs.push_back(base + (uint64_t)(i * N + j) * 4);

    return addrs;
}

// ── build a default 3-level hierarchy ────────────────────────────────────────

static std::unique_ptr<CacheHierarchy> build_default_hierarchy(ReplacementPolicy pol) {
    auto h = std::make_unique<CacheHierarchy>();

    // L1: 32 KB, 8-way, 64B blocks
    h->add_level(std::make_unique<CacheLevel>(
        "L1", 32*1024, 8, 64, pol, WritePolicy::WRITE_BACK, 4));

    // L2: 256 KB, 8-way, 64B blocks
    h->add_level(std::make_unique<CacheLevel>(
        "L2", 256*1024, 8, 64, pol, WritePolicy::WRITE_BACK, 12));

    // L3: 4 MB, 16-way, 64B blocks
    h->add_level(std::make_unique<CacheLevel>(
        "L3", 4*1024*1024, 16, 64, pol, WritePolicy::WRITE_BACK, 36));

    return h;
}

// ── run a simulation and print results ───────────────────────────────────────

static void run_simulation(const std::string& label,
                           CacheHierarchy&    hier,
                           const std::vector<uint64_t>& addrs,
                           bool is_write = false)
{
    std::cout << "\n>>> Workload: " << label << "\n";
    hier.reset_stats();
    for (uint64_t addr : addrs)
        hier.access(addr, is_write);
    hier.print_stats();
    std::cout << "AMAT: " << hier.amat(200) << " cycles\n";
}

// ── main ──────────────────────────────────────────────────────────────────────

int main(int argc, char* argv[]) {
    std::cout << "╔══════════════════════════════════════════╗\n";
    std::cout << "║    Cache Hierarchy Simulator  v0.1       ║\n";
    std::cout << "╚══════════════════════════════════════════╝\n\n";

    // ── Mode 1: trace file supplied on command line ──
    if (argc >= 2) {
        std::string trace_path = argv[1];
        std::string pol_str    = (argc >= 3) ? argv[2] : "lru";

        ReplacementPolicy pol = ReplacementPolicy::LRU;
        if      (pol_str == "fifo")   pol = ReplacementPolicy::FIFO;
        else if (pol_str == "lfu")    pol = ReplacementPolicy::LFU;
        else if (pol_str == "random") pol = ReplacementPolicy::RANDOM;

        std::cout << "Trace file : " << trace_path << "\n";
        std::cout << "Policy     : " << pol_str   << "\n\n";

        auto hier = build_default_hierarchy(pol);
        hier->print_config();

        TraceReader reader(trace_path);
        MemAccess   acc;
        uint64_t    count = 0;
        while ((acc = reader.next()).valid) {
            hier->access(acc.address, acc.is_write);
            ++count;
        }
        std::cout << "Total accesses: " << count << "\n";
        hier->print_stats();
        std::cout << "AMAT: " << hier->amat(200) << " cycles\n";
        return 0;
    }

    // ── Mode 2: built-in synthetic workloads ──
    std::cout << "No trace file supplied — running built-in synthetic workloads.\n";
    std::cout << "Usage: ./cache_sim <trace_file> [lru|fifo|lfu|random]\n\n";

    auto matrix_trace = generate_matrix_trace(64);

    // Split into row-major (first half) and col-major (second half)
    std::vector<uint64_t> row_major(matrix_trace.begin(),
                                    matrix_trace.begin() + matrix_trace.size()/2);
    std::vector<uint64_t> col_major(matrix_trace.begin() + matrix_trace.size()/2,
                                    matrix_trace.end());

    // Compare LRU vs FIFO vs LFU on both access patterns
    std::vector<std::pair<std::string, ReplacementPolicy>> policies = {
        {"LRU",    ReplacementPolicy::LRU},
        {"FIFO",   ReplacementPolicy::FIFO},
        {"LFU",    ReplacementPolicy::LFU},
        {"RANDOM", ReplacementPolicy::RANDOM},
    };

    for (auto& [name, pol] : policies) {
        std::cout << "\n========================================\n";
        std::cout << "Policy: " << name << "\n";
        std::cout << "========================================";

        auto hier = build_default_hierarchy(pol);
        hier->print_config();

        run_simulation("Matrix 64×64  [row-major / cache-friendly]",
                       *hier, row_major, false);
        run_simulation("Matrix 64×64  [col-major / cache-unfriendly]",
                       *hier, col_major, false);
    }

    return 0;
}
