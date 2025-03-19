#!/bin/bash

slow=(0 0.1 0.2 0.3 0.4 0.5 0.6 0.7 0.8 0.9 1.0)
low_cpu=(0 0.1 0.2 0.3 0.4 0.5 0.6 0.7 0.8 0.9 1.0)
blk_time=(50 5000 10000 20000 50000 600000)

mkdir -p exp_logs2
for cpu in "${low_cpu[@]}"; do
    for sl in "${slow[@]}"; do
        for blk in "${blk_time[@]}"; do
            echo "Running make with LOW_CPU=$cpu and SLOW=$sl and BLK_TIME=$blk"
            make run -e LOW_CPU=$cpu SLOW=$sl TOTAL_NODES=50 BLK_TIME=$blk TTX_TIME=5000 BLK_LIMIT=200
            # mv graphs graphs_cpu_${cpu}_sl_${sl}
            mv logs exp_logs2/logs_cpu_${cpu}_sl_${sl}_blk_${blk}
        done
    done
done