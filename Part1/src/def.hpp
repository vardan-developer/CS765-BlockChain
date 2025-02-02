#ifndef DEF_H
#define DEF_H

#include <vector>
#include <string>
#include <queue>
#include <iostream>
#include <fstream>
#include <stack>
#include <set>
#include <unordered_map>
#include <algorithm>
#include <numeric>
#include <unordered_set>
#include <algorithm>
#include <random>
#include <queue>

using txnId_t = uint64_t;
using minerId_t = uint64_t;
using blockId_t = uint64_t;

const int MB = 1000000;
const double BLOCK_INTER_ARRIVAL_TIME = 600;
const uint64_t MINING_REWARD = 50;
const double TXN_INTER_ARRIVAL_TIME = 60;  //TODO: Take this as command line argument
const int NUM_MINERS = 10;
#endif