import os
import re
import matplotlib.pyplot as plt
import numpy as np

# Hardcoded parameter values
LOW_CPU_VALUES = [0, 0.1, 0.2, 0.3, 0.4, 0.5, 0.6, 0.7, 0.8, 0.9, 1.0]
SLOW_VALUES = [0.1, 0.2, 0.3, 0.4, 0.5, 0.6, 0.7, 0.8, 0.9]  # Excluding 0 and 1.0
BLK_TIME_VALUES = [50, 5000, 10000, 20000, 50000]

def parse_logs(parent_directory):
    data = {}
    
    for folder in os.listdir(parent_directory):
        match = re.search(r'logs_cpu_(\d\.\d+)_sl_(\d\.\d+)_blk_(\d+)', folder)
        if match:
            cpu, sl, blk_time = map(float, match.groups())
            blk_time = int(blk_time)
            
            if cpu in LOW_CPU_VALUES and sl in SLOW_VALUES and blk_time in BLK_TIME_VALUES:
                folder_path = os.path.join(parent_directory, folder)
                slow_ratios = []
                fast_ratios = []
                
                for filename in os.listdir(folder_path):
                    if filename.startswith("miner-") and filename.endswith(".logs"):
                        with open(os.path.join(folder_path, filename), 'r') as f:
                            content = f.read()
                            gen_match = re.search(r'Total Blocks Generated: (\d+)', content)
                            main_match = re.search(r'Total Blocks in Main Chain: (\d+)', content)
                            miner_type_match = re.search(r'(Fast|Slow), (lowCpu|highCpu)', content)
                            
                            if gen_match and main_match and miner_type_match:
                                total_gen = int(gen_match.group(1))
                                total_main = int(main_match.group(1))
                                miner_type = miner_type_match.group(1)
                                
                                if total_gen > 0:  # Avoid division by zero
                                    ratio = total_main / total_gen
                                    if miner_type == "Slow":
                                        slow_ratios.append(ratio)
                                    else:
                                        fast_ratios.append(ratio)

                                else:
                                    if miner_type == "Slow":
                                        slow_ratios.append(0)
                                    else:
                                        fast_ratios.append(0)
                
                if slow_ratios or fast_ratios:
                    key = (cpu, blk_time, sl)
                    data[key] = {
                        "slow": sum(slow_ratios) / len(slow_ratios) if slow_ratios else None,
                        "fast": sum(fast_ratios) / len(fast_ratios) if fast_ratios else None
                    }
    
    return data

def plot_data(data, output_dir="plots_avg_ratio_vs_slow"):
    os.makedirs(output_dir, exist_ok=True)
    unique_pairs = sorted(set((cpu, blk) for cpu, blk, _ in data.keys()))
    
    for cpu, blk in unique_pairs:
        slow_vals = sorted(set(sl for (c, b, sl) in data.keys() if c == cpu and b == blk))
        slow_ratios = [data[(cpu, blk, sl)]["slow"] for sl in slow_vals if data[(cpu, blk, sl)]["slow"] is not None]
        fast_ratios = [data[(cpu, blk, sl)]["fast"] for sl in slow_vals if data[(cpu, blk, sl)]["fast"] is not None]
        
        plt.figure(figsize=(8, 5))
        plt.plot(slow_vals, slow_ratios, marker='o', linestyle='-', label='Slow Miners')
        plt.plot(slow_vals, fast_ratios, marker='s', linestyle='-', label='Fast Miners')
        
        plt.xlabel("Slow Miners Ratio")
        plt.ylabel("Average Ratio of Blocks in Main Chain")
        plt.title(f"Avg Ratio of Main Chain Blocks vs. Slow Miners Ratio\nlowCpu_miners_ratio={cpu}, blkTime={blk}")
        plt.legend()
        plt.grid(True)
        
        plot_filename = os.path.join(output_dir, f"plot_cpu_{cpu}_blk_{blk}.png")
        plt.savefig(plot_filename)
        plt.close()
        print(f"Saved plot: {plot_filename}")

parent_directory = "exp_logs2"  # Change this if your logs are stored elsewhere
data = parse_logs(parent_directory)
plot_data(data)
