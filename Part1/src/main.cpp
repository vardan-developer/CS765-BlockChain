#include "parser.hpp"
#include "sim.hpp"
#include "utils.hpp"

std::vector<std::vector<std::pair<minerID_t, std::pair<int, int> > > > networkTopology;
std::set<minerID_t> highCPUMiners;
// networkTopology[i][j] = (neighborID, (rho_ij, c_ij))
// latency = rho_ij + |m| * 1000 / c_ij + d_ij -> in milliseconds
// d_ij is randomly chosen from a exponential dist with mean 96*1000Kbits/c_ij (in milliseconds)
// rho_ij is randomly chosen from a uniform dist between 10ms to 500ms
// c_ij is 100*1024 Kbps if both i and j are fast else 5*1024 Kbps
int main(int argc, char* argv[]){
    ProgramSettings settings;
    std::string errorMsg;

    if(!parseArgs(argc, argv, settings, errorMsg)){
        std::cerr << errorMsg << std::endl;
        return 1;
    }

    networkTopology = generateNetworkTopology(settings.totalNodes, settings.z0);
    highCPUMiners = getHighCPUMiners(settings.totalNodes, settings.z1);
    Simulator simulator(settings.totalNodes, settings.Ttx, settings.I);
}