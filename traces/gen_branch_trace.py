"""
Generates synthetic branch traces for the predictor.
Format: PC_address taken(1) or not_taken(0)

Usage:
    python3 traces/gen_branch_trace.py matrix > traces/branch_matrix.txt
    python3 traces/gen_branch_trace.py bfs    > traces/branch_bfs.txt
"""
import sys
import random

def matrix_branches(N=128):
    """Loops in matrix multiply — highly predictable."""
    lines = []
    # 3 nested loops — outer 2 almost always taken, inner exits regularly
    for i in range(N):
        for j in range(N):
            for k in range(N):
                pc = 0x4000 + (k % 64) * 4
                taken = (k < N - 1)   # loop back taken except last iter
                lines.append(f"0x{pc:x} {1 if taken else 0}")
    return lines

def bfs_branches(num_nodes=256, edges=8):
    """BFS conditionals — unpredictable visited checks."""
    lines = []
    visited = set()
    queue   = list(range(32))
    for node in queue:
        for _ in range(edges):
            neighbor = random.randint(0, num_nodes - 1)
            pc       = 0x5000 + (node % 32) * 4
            taken    = neighbor not in visited   # unpredictable
            if taken:
                visited.add(neighbor)
            lines.append(f"0x{pc:x} {1 if taken else 0}")
    return lines

def sort_branches(N=512):
    """Bubble sort comparisons — semi-predictable."""
    arr = list(range(N))
    random.shuffle(arr)
    lines = []
    for i in range(N):
        for j in range(N - i - 1):
            pc    = 0x6000
            taken = arr[j] > arr[j+1]
            if taken:
                arr[j], arr[j+1] = arr[j+1], arr[j]
            lines.append(f"0x{pc:x} {1 if taken else 0}")
    return lines

workload = sys.argv[1] if len(sys.argv) > 1 else "matrix"
if   workload == "matrix": lines = matrix_branches(32)
elif workload == "bfs":    lines = bfs_branches()
elif workload == "sort":   lines = sort_branches(128)
else:
    print(f"Unknown: {workload}", file=sys.stderr)
    sys.exit(1)

print(f"# Branch trace: {workload}  branches={len(lines)}")
print('\n'.join(lines))

