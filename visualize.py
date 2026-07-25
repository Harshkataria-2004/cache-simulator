import csv
import matplotlib.pyplot as plt
import numpy as np

# Read results
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

workloads = ['matrix', 'bfs', 'sort']
policies  = ['lru', 'fifo', 'lfu', 'random']
colors    = ['#2196F3', '#FF5722', '#4CAF50']
pol_colors= ['#3498db', '#e74c3c', '#2ecc71', '#f39c12']

fig, axes = plt.subplots(2, 2, figsize=(16, 11))
fig.suptitle('Cache Hierarchy + Branch Predictor Simulator',
             fontsize=15, fontweight='bold')

# Graph 1 — AMAT by workload (LRU only)
amat_vals = [data[w]['lru']['amat'] for w in workloads]
axes[0][0].bar(workloads, amat_vals, color=colors)
axes[0][0].set_title('AMAT by Workload (LRU policy)')
axes[0][0].set_ylabel('AMAT (cycles)')
axes[0][0].set_xlabel('Workload')
for i, v in enumerate(amat_vals):
    axes[0][0].text(i, v + 0.2, f'{v}', ha='center', fontweight='bold')

# Graph 2 — L1, L2, L3 hit rates side by side (LRU only)
x = np.arange(len(workloads))
width = 0.25
l1_rates = [data[w]['lru']['l1'] for w in workloads]
l2_rates = [data[w]['lru']['l2'] for w in workloads]
l3_rates = [data[w]['lru']['l3'] for w in workloads]

bars1 = axes[0][1].bar(x - width, l1_rates, width, label='L1', color='#2196F3')
bars2 = axes[0][1].bar(x,         l2_rates, width, label='L2', color='#FF5722')
bars3 = axes[0][1].bar(x + width, l3_rates, width, label='L3', color='#4CAF50')
axes[0][1].set_title('Hit Rate by Cache Level (LRU)')
axes[0][1].set_ylabel('Hit Rate (%)')
axes[0][1].set_xlabel('Workload')
axes[0][1].set_xticks(x)
axes[0][1].set_xticklabels(workloads)
axes[0][1].legend()
for bar in bars1:
    h = bar.get_height()
    if h > 0:
        axes[0][1].text(bar.get_x()+bar.get_width()/2, h+0.5,
                        f'{h:.1f}%', ha='center', fontsize=7, fontweight='bold')
for bar in bars2:
    h = bar.get_height()
    if h > 0:
        axes[0][1].text(bar.get_x()+bar.get_width()/2, h+0.5,
                        f'{h:.1f}%', ha='center', fontsize=7, fontweight='bold')

# Graph 3 — Policy comparison on matrix (AMAT)
matrix_amats = [data['matrix'][p]['amat'] for p in policies]
axes[1][0].bar(policies, matrix_amats, color=pol_colors)
axes[1][0].set_title('Policy Comparison — Matrix Workload (AMAT)')
axes[1][0].set_ylabel('AMAT (cycles)')
axes[1][0].set_xlabel('Replacement Policy')
for i, v in enumerate(matrix_amats):
    axes[1][0].text(i, v + 0.05, f'{v}', ha='center', fontweight='bold')

# Graph 4 — Branch predictor accuracy
branch_workloads  = ['matrix', 'bfs', 'sort']
branch_accuracies = [100.00, 60.94, 68.55]
miss_rates = [100 - data[w]['lru']['l1'] for w in workloads]

axes[1][1].scatter(miss_rates,
                   [100 - a for a in branch_accuracies],
                   color=colors, s=200, zorder=5)
for i, w in enumerate(workloads):
    axes[1][1].annotate(w,
                        (miss_rates[i], 100 - branch_accuracies[i]),
                        textcoords="offset points",
                        xytext=(10, 5), fontsize=11)
axes[1][1].set_title('Cache Miss Rate vs Branch Misprediction Rate')
axes[1][1].set_xlabel('L1 Miss Rate (%)')
axes[1][1].set_ylabel('Branch Misprediction Rate (%)')
axes[1][1].grid(True, alpha=0.3)

plt.tight_layout()
plt.savefig('results.png', dpi=150, bbox_inches='tight')
print("Graph saved to results.png")

