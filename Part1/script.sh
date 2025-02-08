#!/bin/bash

slow=(0 0.1 0.2 0.3 0.4 0.5 0.6 0.7 0.8 0.9 1.0)
low_cpu=(0 0.2 0.4 0.6 0.8 1.0)

for cpu in "${low_cpu[@]}"; do
    for sl in "${slow[@]}"; do
        echo "Running make with LOW_CPU=$cpu and SLOW=$sl"
        make run -e LOW_CPU=$cpu SLOW=$sl TOTAL_NODES=50 BLK_TIME=600 TTX_TIME=5000 BLK_LIMIT=200
        # mv graphs graphs_cpu_${cpu}_sl_${sl}
        mv logs logs_cpu_${cpu}_sl_${sl}
    done
done