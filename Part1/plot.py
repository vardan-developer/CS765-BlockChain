import os
import re
import numpy as np
import matplotlib.pyplot as plt

# Parameters from the script
slow_values = [0, 0.1, 0.2, 0.3, 0.4, 0.5, 0.6, 0.7, 0.8, 0.9, 1.0]
low_cpu_values = [0, 0.2, 0.4, 0.6, 0.8, 1.0]

# Data storage: {low_cpu_value: {slow_value: [ratios]}}
lowcpu_data = {cpu: {s: [] for s in slow_values} for cpu in low_cpu_values}
highcpu_data = {cpu: {s: [] for s in slow_values} for cpu in low_cpu_values}

# Regex patterns
ratio_pattern = re.compile(r"Ratio .*: ([\d\.]+)")
category_pattern = re.compile(r"(Slow|Fast), (lowCpu|highCpu)")

# Process log folders
for cpu in low_cpu_values:
    for sl in slow_values:
        folder = f"logs_cpu_{cpu}_sl_{sl}"
        if not os.path.exists(folder):
            print(f"Warning: {folder} not found, skipping...")
            continue
        
        ratios_lowcpu = []
        ratios_highcpu = []
        
        # Process log files in folder
        for log_file in os.listdir(folder):
            log_path = os.path.join(folder, log_file)
            if not os.path.isfile(log_path):
                continue

            with open(log_path, 'r') as file:
                lines = file.readlines()
                if len(lines) < 2:
                    continue  # Ignore incomplete logs
                
                ratio_match = ratio_pattern.search(lines[-2])
                category_match = category_pattern.search(lines[-1])
                
                if ratio_match and category_match:
                    ratio = float(ratio_match.group(1))
                    if ratio < 0:
                        continue
                    _, cpu_type = category_match.groups()

                    if "lowCpu" in cpu_type:
                        ratios_lowcpu.append(ratio)
                    else:
                        ratios_highcpu.append(ratio)

        # Compute averages
        if ratios_lowcpu:
            lowcpu_data[cpu][sl] = np.mean(ratios_lowcpu)
        if ratios_highcpu:
            highcpu_data[cpu][sl] = np.mean(ratios_highcpu)

# Plot function
def plot_graph(data, title, filename, low_cpu_values=low_cpu_values):
    plt.figure(figsize=(10, 6))
    
    for cpu in low_cpu_values:
        avg_ratios = [data[cpu][s] if isinstance(data[cpu][s], float) else 0 for s in slow_values]
        plt.plot(slow_values, avg_ratios, marker='o', label=f"Low CPU = {cpu}")

    plt.xlabel("Slow Value")
    plt.ylabel("Average Ratio")
    plt.title(title)
    plt.legend()
    plt.grid()
    plt.savefig(filename)
    plt.show()

# Generate graphs
plot_graph(lowcpu_data, "Average Ratio of Blocks in main chain vs Slow (LowCPU Miners)", "lowcpu_miners.png", low_cpu_values[1:])
plot_graph(highcpu_data, "Average Ratio of Blocks in main chain vs Slow (HighCPU Miners)", "highcpu_miners.png", low_cpu_values[:-1])
