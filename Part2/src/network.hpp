#include "def.hpp"
#include "utils.hpp"

class Network {
    public:
    std::vector<std::vector<std::pair<int, int>>> networkTopology;
    Network(int honest, int malicious) {
        networkTopology = generateNetworkTopology(honest+malicious, float(honest)/(honest + malicious));
    }
    float getLatency(int i, int j) {
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