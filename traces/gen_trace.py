import sys
import random

BASE = 0x100000

def matrix_multiply(N=128):
    lines = []
    a_base = BASE
    b_base = BASE + N*N*4
    c_base = BASE + 2*N*N*4
    for i in range(N):
        for j in range(N):
            for k in range(N):
                lines.append(f"R 0x{a_base + (i*N+k)*4:x}")
                lines.append(f"R 0x{b_base + (k*N+j)*4:x}")
                lines.append(f"R 0x{c_base + (i*N+j)*4:x}")
                lines.append(f"W 0x{c_base + (i*N+j)*4:x}")
    return lines

def bubble_sort(N=1024):
    lines = []
    arr_base = BASE
    for i in range(N):
        for j in range(N - i - 1):
            lines.append(f"R 0x{arr_base + j*4:x}")
            lines.append(f"R 0x{arr_base + (j+1)*4:x}")
            if random.random() < 0.5:
                lines.append(f"W 0x{arr_base + j*4:x}")
                lines.append(f"W 0x{arr_base + (j+1)*4:x}")
    return lines

def bfs(num_nodes=10000, edges_per_node=16):
    lines = []
    adj_base   = BASE
    queue_base = BASE + num_nodes * edges_per_node * 4
    visited    = BASE + queue_base + num_nodes * 4
    queue = list(range(min(num_nodes, 64)))
    visited_set = set(queue)
    for node in queue:
        for e in range(edges_per_node):
            neighbor = random.randint(0, num_nodes - 1)
            lines.append(f"R 0x{adj_base + (node*edges_per_node+e)*4:x}")
            lines.append(f"R 0x{visited + neighbor*4:x}")
            if neighbor not in visited_set:
                visited_set.add(neighbor)
                lines.append(f"W 0x{visited + neighbor*4:x}")
                lines.append(f"W 0x{queue_base + len(visited_set)*4:x}")
    return lines

def pointer_chase(num_nodes=131072, passes=3):
    lines = []
    nodes = list(range(num_nodes))
    random.shuffle(nodes)
    node_base = BASE
    for _ in range(passes):
        current = 0
        for _ in range(num_nodes):
            lines.append(f"R 0x{node_base + current * 8 + 4:x}")
            current = nodes[current]
    return lines

def bfs_large(num_nodes=10000, edges_per_node=8):
    lines = []
    adj_base   = BASE
    queue_base = BASE + num_nodes * edges_per_node * 4
    visited    = BASE + queue_base + num_nodes * 4
    adj = {i: random.sample(range(num_nodes), edges_per_node)
           for i in range(num_nodes)}
    queue = [0]
    visited_set = {0}
    while queue:
        node = queue.pop(0)
        for idx, neighbor in enumerate(adj[node]):
            lines.append(f"R 0x{adj_base + (node*edges_per_node+idx)*4:x}")
            lines.append(f"R 0x{visited + neighbor*4:x}")
            if neighbor not in visited_set:
                visited_set.add(neighbor)
                lines.append(f"W 0x{visited + neighbor*4:x}")
                lines.append(f"W 0x{queue_base + len(queue)*4:x}")
                queue.append(neighbor)
    return lines

workload = sys.argv[1] if len(sys.argv) > 1 else "matrix"

if workload == "matrix":
    lines = matrix_multiply(128)
elif workload == "sort":
    lines = bubble_sort(256)
elif workload == "bfs":
    lines = bfs()
elif workload == "pointer":
    lines = pointer_chase()
elif workload == "bfs_large":
    lines = bfs_large()
else:
    print(f"Unknown workload: {workload}", file=sys.stderr)
    sys.exit(1)

print(f"# Trace: {workload}  accesses={len(lines)}")
print('\n'.join(lines))
