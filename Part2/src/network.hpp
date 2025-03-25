#ifndef NETWORK_H
#define NETWORK_H

#include "def.hpp"
#include "utils.hpp"

class Network {
    public:
    std::vector<std::vector<std::pair<int, int>>> networkTopology;          // Adjaceny Matrix
    Network(int totalNodes, double maliciousFraction, bool malicious): networkTopology(malicious ? generateMaliciousNetworkTopology(totalNodes * maliciousFraction): generateNetworkTopology(totalNodes, maliciousFraction * totalNodes)) {
        std::cout << "Network topology generated" << std::endl;
        for (int i = 0; i < networkTopology.size(); i++) {
            std::cout << "Node " << i << ": ";
            for (int j = 0; j < networkTopology[i].size(); j++) {
                if(networkTopology[i][j].first >= 0) std::cout << j << "(" << networkTopology[i][j].first << ","<<networkTopology[i][j].second<<"), ";
            }
            std::cout << std::endl;
        }
    }
    float getLatency(int i, int j) {
        // std::cout << "data: " << 90.0 * Kb * 1000/networkTopology[i][j].second << std::endl;
        return networkTopology[i][j].first + ceil(getExponentialRandom(96.0 * Kb * 1000/networkTopology[i][j].second)) + ceil((Kb * 8.0 * 1000)/networkTopology[i][j].second);
    }
    std::vector<minerID_t> getNeighbors(int i) {
        std::vector<minerID_t> neighbors;
        for (minerID_t neighbor_t = 0; neighbor_t < networkTopology[i].size(); neighbor_t++) {
            if (networkTopology[i][neighbor_t].first >= 0) neighbors.push_back(neighbor_t);
        }
        return neighbors;
    }
};

#endif