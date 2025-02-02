#ifndef UTILS_H
#define UTILS_H

#include "def.hpp"


double getExponentialRandom(double mean);
double getUniformRandom(double a, double b);
bool is_connected(const std::vector<std::unordered_set<int> >  &adj);
std::vector<std::vector<int> > generate_graph(int n);

struct pair_hash {
    std::size_t operator()(const std::pair<int, int>& p) const {
        return std::hash<int>()(p.first) ^ (std::hash<int>()(p.second) << 1);
    }
};

#endif