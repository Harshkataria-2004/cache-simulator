#!/bin/bash
python3 traces/gen_trace.py matrix > traces/matrix_trace.txt

echo "policy,trial,amat" > trial_results.csv

for policy in lru fifo lfu random; do
    for trial in $(seq 1 10); do
        output=$(./cache_sim traces/matrix_trace.txt $policy)
        amat=$(echo "$output" | grep -oP 'AMAT: \K[0-9.]+')
        echo "$policy,$trial,$amat" >> trial_results.csv
    done
done

echo "Done!"
cat trial_results.csv

