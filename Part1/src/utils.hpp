#ifndef UTILS_H
#define UTILS_H

#include <vector>
#include <unordered_set>
#include <algorithm>
#include <random>
#include <queue>


double getExponentialRandom(double mean);
double getUniformRandom(double a, double b);
bool is_connected(const std::vector<std::unordered_set<int> >  &adj);
std::vector<std::vector<int> > generate_graph(int n);

#endif