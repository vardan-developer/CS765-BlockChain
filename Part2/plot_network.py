import networkx as nx
import matplotlib.pyplot as plt

def read_network(file_path):
    """ Reads a network topology file and returns an adjacency list. """
    adjacency_list = {}
    with open(file_path, 'r') as file:
        lines = file.readlines()[1:]  # Skip header
        for line in lines:
            line = line.strip()
            if not line.startswith("Node"):  # Ensure it's a valid line
                continue
            parts = line.split(":")
            node = int(parts[0].replace("Node", "").strip())  # Extract node number
            neighbors = list(map(int, parts[1].strip().split())) if parts[1].strip() else []
            adjacency_list[node] = neighbors
    return adjacency_list

# Load networks
honest_network = read_network("logs/honestNetworkTopology.log")
malicious_network = read_network("logs/maliciousNetworkTopology.log")

# Create graphs
G_honest = nx.Graph()
for node, neighbors in honest_network.items():
    for neighbor in neighbors:
        G_honest.add_edge(node, neighbor)

G_malicious = nx.Graph()
for node, neighbors in malicious_network.items():
    for neighbor in neighbors:
        G_malicious.add_edge(node, neighbor)

# Identify nodes
malicious_nodes = set(G_malicious.nodes())  # Nodes part of malicious network
all_nodes = set(G_honest.nodes())  # All nodes (honest + malicious)
honest_nodes = all_nodes - malicious_nodes  # Honest nodes

# Generate positions for consistency across plots
pos = nx.spring_layout(G_honest, seed=42)

### Plot 1: Honest Network ###
plt.figure(figsize=(10, 7))
nx.draw_networkx_nodes(G_honest, pos, nodelist=honest_nodes, node_color="blue", label="Honest Nodes", node_size=500)
nx.draw_networkx_nodes(G_honest, pos, nodelist=malicious_nodes, node_color="red", label="Malicious Nodes", node_size=500)
nx.draw_networkx_edges(G_honest, pos, edge_color="gray", style="solid", alpha=0.6, width=1.2)
nx.draw_networkx_labels(G_honest, pos, font_size=8, font_color="white")

plt.legend(loc="best")
plt.title("Network Topology")
plt.savefig("graphs/honestGraph.png")  # Save image

pos = nx.spring_layout(G_malicious, seed=42)  # Update positions for malicious network
### Plot 2: Malicious Network ###
plt.figure(figsize=(10, 7))
nx.draw_networkx_nodes(G_malicious, pos, nodelist=malicious_nodes, node_color="red", label="Malicious Nodes", node_size=500)
nx.draw_networkx_edges(G_malicious, pos, edge_color="gray", style="solid", alpha=0.6, width=1.2)
nx.draw_networkx_labels(G_malicious, pos, font_size=8, font_color="white")

plt.title("Malicious Network Topology")
plt.savefig("graphs/maliciousGraph.png")  # Save image