#!/bin/bash
python3 traces/gen_trace.py matrix > traces/matrix_trace.txt

echo "policy,trial,seed,amat" > trial_results.csv

seeds=(42 123 456 789 1011 1213 1415 1617 1819 2021)

for policy in lru fifo lfu random; do
    for i in $(seq 0 9); do
        seed=${seeds[$i]}
        trial=$((i+1))
        output=$(./cache_sim traces/matrix_trace.txt $policy $seed)
        amat=$(echo "$output" | grep -oP 'AMAT: \K[0-9.]+')
        echo "$policy,$trial,$seed,$amat" >> trial_results.csv
    done
done

echo "Done!"
cat trial_results.csv