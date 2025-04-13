#!/home/atharva/Desktop/AR/Packing-Color/Assn3/ar/bin/ python3
import json
import sys
import matplotlib.pyplot as plt
import numpy as np

def extract_json_data(file_path='data.txt'):
    """Extract JSON data from the specified file between DATA_POINTS_JSON_START and DATA_POINTS_JSON_END markers."""
    try:
        with open(file_path, 'r') as file:
            content = file.read()
            
            # Find the JSON data between the markers
            start_marker = "DATA_POINTS_JSON_START"
            end_marker = "DATA_POINTS_JSON_END"
            
            start_index = content.find(start_marker)
            end_index = content.find(end_marker)
            
            if start_index == -1 or end_index == -1:
                print("Error: Could not find the required markers in the file.")
                sys.exit(1)
                
            # Extract the JSON data (everything between the markers)
            json_data = content[start_index + len(start_marker):end_index].strip()
            
            if not json_data:
                print("No data found in the file. Make sure the file contains data between DATA_POINTS_JSON_START and DATA_POINTS_JSON_END markers.")
                sys.exit(1)
            
            try:
                return json.loads(json_data)
            except json.JSONDecodeError as e:
                print(f"Error parsing JSON data: {e}")
                sys.exit(1)
                
    except FileNotFoundError:
        print(f"Error: File '{file_path}' not found.")
        sys.exit(1)

def plot_total_value_locked(data_points):
    """Plot total value locked for both tokens."""
    iterations = [dp['iteration'] for dp in data_points]
    tvl_a = [float(dp['totalValueLockedA']) for dp in data_points]
    tvl_b = [float(dp['totalValueLockedB']) for dp in data_points]
    
    plt.figure(figsize=(10, 6))
    plt.plot(iterations, tvl_a, label='Token A', markersize=3)
    plt.plot(iterations, tvl_b, label='Token B', markersize=3)
    plt.xlabel('Iteration')
    plt.ylabel('Total Value Locked')
    plt.title('Total Value Locked Over Time')
    plt.legend()
    plt.grid(True)
    plt.savefig('total_value_locked.png')
    plt.close()

def plot_trading_volume(data_points):
    """Plot trading volume for both tokens."""
    iterations = [dp['iteration'] for dp in data_points]
    volume_a = [dp['tokenATradingVolume'] for dp in data_points]
    volume_b = [dp['tokenBTradingVolume'] for dp in data_points]
    
    plt.figure(figsize=(10, 6))
    plt.plot(iterations, volume_a, label='Token A', markersize=3)
    plt.plot(iterations, volume_b, label='Token B', markersize=3)
    plt.xlabel('Iteration')
    plt.ylabel('Trading Volume')
    plt.title('Trading Volume Over Time')
    plt.legend()
    plt.grid(True)
    plt.savefig('trading_volume.png')
    plt.close()

def plot_fee_collected(data_points):
    """Plot fee collected for both tokens."""
    iterations = [dp['iteration'] for dp in data_points]
    fee_a = [dp['tokenAFeeCollected'] for dp in data_points]
    fee_b = [dp['tokenBFeeCollected'] for dp in data_points]
    
    plt.figure(figsize=(10, 6))
    plt.plot(iterations, fee_a, label='Token A', markersize=3)
    plt.plot(iterations, fee_b, label='Token B', markersize=3)
    plt.xlabel('Iteration')
    plt.ylabel('Fee Collected')
    plt.title('Fee Collected Over Time')
    plt.legend()
    plt.grid(True)
    plt.savefig('fee_collected.png')
    plt.close()

def plot_reserve_ratio(data_points):
    """Plot reserve ratio over time with logarithmic scale."""
    iterations = [dp['iteration'] for dp in data_points]
    reserve_ratio = [dp['reserveRatio'] for dp in data_points]
    
    plt.figure(figsize=(10, 6))
    plt.semilogy(iterations, reserve_ratio, markersize=3)
    plt.axhline(y=1, color='r', linestyle='--', alpha=0.5)
    plt.xlabel('Iteration')
    plt.ylabel('Reserve Ratio (log scale)')
    plt.title('Reserve Ratio Over Time')
    plt.grid(True, which="both", ls="-", alpha=0.2)
    plt.grid(True, which="minor", ls=":", alpha=0.2)
    plt.minorticks_on()
    plt.savefig('reserve_ratio.png')
    plt.close()

def plot_spot_price(data_points):
    """Plot spot price over time with logarithmic scale."""
    iterations = [dp['iteration'] for dp in data_points]
    spot_price = [dp['spotPrice'] for dp in data_points]
    
    plt.figure(figsize=(10, 6))
    plt.semilogy(iterations, spot_price, markersize=3)
    plt.axhline(y=1, color='r', linestyle='--', alpha=0.5)
    plt.xlabel('Iteration')
    plt.ylabel('Spot Price (log scale)')
    plt.title('Spot Price Over Time')
    plt.grid(True, which="both", ls="-", alpha=0.2)
    plt.grid(True, which="minor", ls=":", alpha=0.2)
    plt.minorticks_on()
    plt.savefig('spot_price.png')
    plt.close()

def plot_slippage(data_points):
    """Plot slippage over time."""
    iterations = [dp['iteration'] for dp in data_points]
    slippage = [dp['slippage'] if dp['slippage'] is not None else 0 for dp in data_points]
    
    plt.figure(figsize=(10, 6))
    plt.plot(iterations, slippage, markersize=3)
    plt.axhline(y=0, color='r', linestyle='--', alpha=0.5)
    plt.xlabel('Iteration')
    plt.ylabel('Slippage (%)')
    plt.title('Slippage Over Time')
    plt.grid(True)
    plt.savefig('slippage.png')
    plt.close()

def plot_lp_token_distribution(data_points):
    """Plot LP token distribution for all LPs on the same graph."""
    if not data_points or 'lpTokenDistribution' not in data_points[0]:
        return
    
    # Get all unique LP addresses
    lp_addresses = set()
    for dp in data_points:
        lp_addresses.update(dp['lpTokenDistribution'].keys())
    
    # Create a single plot for all LPs
    plt.figure(figsize=(12, 8))
    
    # Define a set of colors and markers for different LPs
    colors = ['b', 'g', 'r', 'c', 'm', 'y', 'k']
    markers = ['o', 's', '^', 'D', 'v', 'p', '*']
    
    # Plot each LP's data
    for i, lp in enumerate(lp_addresses):
        iterations = []
        balances = []
        
        for dp in data_points:
            if lp in dp['lpTokenDistribution']:
                iterations.append(dp['iteration'])
                balances.append(float(dp['lpTokenDistribution'][lp]))
        
        if iterations:  # Only plot if we have data for this LP
            color = colors[i % len(colors)]
            plt.plot(iterations, balances, label=f'LP: {lp[:8]}...{lp[-6:]}', 
                     markersize=3, color=color)
    
    plt.xlabel('Iteration')
    plt.ylabel('LP Token Balance')
    plt.title('LP Token Distribution Over Time')
    plt.legend(loc='best')
    plt.grid(True)
    plt.savefig('lp_token_distribution.png')
    plt.close()

def main():
    print("Extracting data from data.txt...")
    data_points = extract_json_data()
    print(f"Successfully extracted {len(data_points)} data points.")
    
    print("Creating plots...")
    plot_total_value_locked(data_points)
    plot_trading_volume(data_points)
    plot_fee_collected(data_points)
    plot_reserve_ratio(data_points)
    plot_spot_price(data_points)
    plot_slippage(data_points)
    plot_lp_token_distribution(data_points)
    
    print("All plots have been saved as PNG files.")

if __name__ == "__main__":
    main() 