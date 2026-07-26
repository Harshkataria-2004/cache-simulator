import subprocess
import csv
import matplotlib.pyplot as plt
import numpy as np

# ── Step 1: vary L1 size ──────────────────────────────────────────────────────

l1_sizes    = [8, 16, 32, 64, 128]   # KB
assoc_vals  = [1, 2, 4, 8, 16]       # ways
workloads   = ['matrix', 'sort', 'bfs_large']

# We'll patch main.cpp temporarily — easier to just call the binary
# with different configs via a wrapper script

# ── Step 2: write a configurable runner ──────────────────────────────────────

runner_code = r"""
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
"""

# Write and compile the sweep binary
with open('src/cache_sweep.cpp', 'w') as f:
    f.write(runner_code)

import os
os.system("g++ -std=c++17 -O2 -Iinclude "
          "src/cache_level.cpp src/cache_hierarchy.cpp "
          "src/trace_reader.cpp src/cache_sweep.cpp "
          "-o cache_sweep 2>/dev/null")

# ── Step 3: sweep L1 size ─────────────────────────────────────────────────────

print("Sweeping L1 size...")
l1_results = {w: [] for w in workloads}

for kb in l1_sizes:
    for w in workloads:
        result = subprocess.run(
            ['./cache_sweep', f'traces/{w}_trace.txt',
             str(kb), '256', '512', '8'],
            capture_output=True, text=True)
        try:
            amat = float(result.stdout.strip())
        except:
            amat = 0
        l1_results[w].append(amat)
        print(f"  L1={kb}KB {w}: AMAT={amat:.2f}")

# ── Step 4: sweep associativity ───────────────────────────────────────────────

print("Sweeping associativity...")
assoc_results = {w: [] for w in workloads}

for assoc in assoc_vals:
    for w in workloads:
        result = subprocess.run(
            ['./cache_sweep', f'traces/{w}_trace.txt',
             '32', '256', '512', str(assoc)],
            capture_output=True, text=True)
        try:
            amat = float(result.stdout.strip())
        except:
            amat = 0
        assoc_results[w].append(amat)
        print(f"  assoc={assoc} {w}: AMAT={amat:.2f}")

# ── Step 5: plot ──────────────────────────────────────────────────────────────

colors = ['#2196F3', '#4CAF50', '#FF5722']
fig, axes = plt.subplots(1, 2, figsize=(14, 6))
fig.suptitle('Cache Sensitivity Analysis', fontsize=14, fontweight='bold')

# Graph 1 — AMAT vs L1 size
for i, w in enumerate(workloads):
    axes[0].plot(l1_sizes, l1_results[w],
                 marker='o', color=colors[i], label=w, linewidth=2)
axes[0].set_title('AMAT vs L1 Cache Size')
axes[0].set_xlabel('L1 Cache Size (KB)')
axes[0].set_ylabel('AMAT (cycles)')
axes[0].set_xticks(l1_sizes)
axes[0].legend()
axes[0].grid(True, alpha=0.3)

# Graph 2 — AMAT vs Associativity
for i, w in enumerate(workloads):
    axes[1].plot(assoc_vals, assoc_results[w],
                 marker='o', color=colors[i], label=w, linewidth=2)
axes[1].set_title('AMAT vs Cache Associativity')
axes[1].set_xlabel('Associativity (ways)')
axes[1].set_ylabel('AMAT (cycles)')
axes[1].set_xticks(assoc_vals)
axes[1].legend()
axes[1].grid(True, alpha=0.3)

plt.tight_layout()
plt.savefig('sensitivity.png', dpi=150, bbox_inches='tight')
print("\nGraph saved to sensitivity.png")

