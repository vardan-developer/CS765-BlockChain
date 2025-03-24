#ifndef UTILS_H
#define UTILS_H

#include "def.hpp"
#include <set>

double getExponentialRandom(double mean);
double getUniformRandom(double a, double b);
bool is_connected(const std::vector<std::unordered_set<int> >  &adj);
std::vector<std::vector<minerID_t> > generate_graph(int n);
std::vector<std::vector<std::pair<int, int> > > generateNetworkTopology(int n, int totalMaliciousNodes);
std::vector<std::vector<std::pair<int, int>>> generateMaliciousNetworkTopology(int totalMaliciousNodes);
std::set<minerID_t> getHighCPUMiners(int n, float z1);

struct pair_hash {
    std::size_t operator()(const std::pair<int, int>& p) const {
        return std::hash<int>()(p.first) ^ (std::hash<int>()(p.second) << 1);
    }
};

class Counter {
public:
    static blockID_t getBlockID();
    static txnID_t getTxnID();
    static id_t getID();
private:
    static blockID_t blockIDCount;  // Declaration
    static txnID_t txnIDCount;      // Declaration
    static id_t idCount;
};



#endif