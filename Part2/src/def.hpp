#ifndef DEFS_HPP
#define DEFS_HPP

#include <string>
#include <vector>
#include <map>
#include <algorithm>
#include <numeric>
#include <unordered_set>
#include <unordered_map>
#include <random>
#include <queue>
#include <iostream>
#include <fstream>
#include <stack>
#include <functional>
#include <cstdint>

using minerID_t = long long int;
using txnID_t = long long int;
using blockID_t = long long int;
using hash_t = size_t;
constexpr long long LONG_LONG_MAX_VAL = std::numeric_limits<long long>::max();

const long long Mb = 1000000;
const int Kb = 1000;
const int COINBASE_REWARD = 50;

#endif
