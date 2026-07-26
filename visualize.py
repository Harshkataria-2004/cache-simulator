import csv
import matplotlib.pyplot as plt
import numpy as np

data = {}
with open('results.csv') as f:
    reader = csv.DictReader(f)
    for row in reader:
        w = row['workload']
        p = row['policy']
        if w not in data:
            data[w] = {}
        data[w][p] = {
            'l1': float(row['l1_hit_rate']),
            'l2': float(row['l2_hit_rate']),
            'l3': float(row['l3_hit_rate']),
            'amat': float(row['amat'])
        }

workloads = ['sort', 'matrix', 'bfs', 'bfs_large', 'pointer']
policies  = ['lru', 'fifo', 'lfu', 'random']
colors    = ['#4CAF50', '#2196F3', '#FF5722', '#9C27B0', '#FF9800']
pol_colors= ['#3498db', '#e74c3c', '#2ecc71', '#f39c12']

fig, axes = plt.subplots(2, 2, figsize=(16, 11))
fig.suptitle('Cache Hierarchy + Branch Predictor Simulator\nAll 5 Workloads',
             fontsize=14, fontweight='bold')

# Graph 1 — AMAT by workload (LRU)
amat_vals = [data[w]['lru']['amat'] for w in workloads]
bars = axes[0][0].bar(workloads, amat_vals, color=colors)
axes[0][0].set_title('AMAT by Workload (LRU Policy)')
axes[0][0].set_ylabel('AMAT (cycles)')
axes[0][0].set_xlabel('Workload')
for i, v in enumerate(amat_vals):
    axes[0][0].text(i, v + 1, f'{v}', ha='center', fontweight='bold', fontsize=9)

# Graph 2 — L1/L2/L3 hit rates
x = np.arange(len(workloads))
width = 0.25
l1 = [data[w]['lru']['l1'] for w in workloads]
l2 = [data[w]['lru']['l2'] for w in workloads]
l3 = [data[w]['lru']['l3'] for w in workloads]

axes[0][1].bar(x - width, l1, width, label='L1', color='#2196F3')
axes[0][1].bar(x,         l2, width, label='L2', color='#FF5722')
axes[0][1].bar(x + width, l3, width, label='L3', color='#4CAF50')
axes[0][1].set_title('Hit Rate by Cache Level (LRU)')
axes[0][1].set_ylabel('Hit Rate (%)')
axes[0][1].set_xlabel('Workload')
axes[0][1].set_xticks(x)
axes[0][1].set_xticklabels(workloads, rotation=15)
axes[0][1].legend()

# Graph 3 — Policy comparison on matrix
matrix_amats = [data['matrix'][p]['amat'] for p in policies]
axes[1][0].bar(policies, matrix_amats, color=pol_colors)
axes[1][0].set_title('Policy Comparison — Matrix Workload')
axes[1][0].set_ylabel('AMAT (cycles)')
axes[1][0].set_xlabel('Replacement Policy')
for i, v in enumerate(matrix_amats):
    axes[1][0].text(i, v + 0.05, f'{v}', ha='center', fontweight='bold')
axes[1][0].text(0.5, -0.15,
    'Random beats LRU by 11% — LRU thrashing on strided access',
    ha='center', transform=axes[1][0].transAxes,
    fontsize=8, style='italic')

# Graph 4 — Branch predictor comparison
branch_workloads  = ['matrix', 'bfs', 'sort']
bit2_acc  = [99.99, 60.93, 68.55]
gshare_acc = [99.99, 67.18, 86.52]
x2 = np.arange(len(branch_workloads))
axes[1][1].bar(x2 - 0.2, bit2_acc,   0.4, label='2-bit',  color='#2196F3')
axes[1][1].bar(x2 + 0.2, gshare_acc, 0.4, label='GShare', color='#FF5722')
axes[1][1].set_title('Branch Predictor: 2-bit vs GShare')
axes[1][1].set_ylabel('Accuracy (%)')
axes[1][1].set_xlabel('Workload')
axes[1][1].set_xticks(x2)
axes[1][1].set_xticklabels(branch_workloads)
axes[1][1].legend()
axes[1][1].set_ylim(50, 105)
for i, (a, b) in enumerate(zip(bit2_acc, gshare_acc)):
    axes[1][1].text(i - 0.2, a + 0.5, f'{a:.1f}%', ha='center', fontsize=7)
    axes[1][1].text(i + 0.2, b + 0.5, f'{b:.1f}%', ha='center', fontsize=7)

plt.tight_layout()
plt.savefig('results.png', dpi=150, bbox_inches='tight')
print("Graph saved to results.png")