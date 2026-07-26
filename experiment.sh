#!/bin/bash
echo "workload,policy,l1_hit_rate,l2_hit_rate,l3_hit_rate,amat" > results.csv

for workload in matrix bfs sort pointer bfs_large; do
    for policy in lru fifo lfu random; do
        output=$(./cache_sim traces/${workload}_trace.txt $policy)
        
        l1_rate=$(echo "$output" | grep "^\[L1\]" | grep -oP 'hit_rate=\K[0-9.]+')
        l2_rate=$(echo "$output" | grep "^\[L2\]" | grep -oP 'hit_rate=\K[0-9.]+')
        l3_rate=$(echo "$output" | grep "^\[L3\]" | grep -oP 'hit_rate=\K[0-9.]+')
        amat=$(echo "$output" | grep -oP 'AMAT: \K[0-9.]+')
        
        echo "$workload,$policy,$l1_rate,$l2_rate,$l3_rate,$amat" >> results.csv
    done
done

echo "Done!"
cat results.csv

