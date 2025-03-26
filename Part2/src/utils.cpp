#include "utils.hpp"

// Random number generation setup
std::random_device rd;
std::mt19937 gen(rd()); 
// std::mt19937 gen(25); // Commented out fixed seed
std::set<minerID_t> maliciousMiners;  // Set to track miners with fast network connections
std::set<minerID_t> honestMiners;  // Set to track miners with slow network connections

// Static counter initialization for block and transaction IDs
blockID_t Counter::blockIDCount = 1;
txnID_t Counter::txnIDCount = 1;
id_t Counter::idCount = 1;


blockID_t Counter::getBlockID(){
    return blockIDCount++;
}

txnID_t Counter::getTxnID(){
    return txnIDCount++;
}

id_t Counter::getID(){
    return idCount++;
}

// Returns a random permutation of integers from 0 to n-1
std::vector<int> getRandomPermutation(int n) {
    std::vector<int> perm(n);
    // Initialize with 0 to n-1
    std::iota(perm.begin(), perm.end(), 0);
    // Shuffle using the existing random generator
    std::shuffle(perm.begin(), perm.end(), gen);
    return perm;
}

// Generates a random number following exponential distribution with given mean
double getExponentialRandom(double mean) {
    if (mean <= 0) {
        throw std::invalid_argument("Mean must be greater than zero.");
    }

    // Create an exponential distribution with lambda = 1/mean
    std::exponential_distribution<double> distribution(1.0 / mean);

    return distribution(gen);
}

// Generates a random number uniformly distributed between a and b
double getUniformRandom(double a, double b) {
    if (a > b) {
        throw std::invalid_argument("Lower bound must be less than upper bound.");
    }
    if (a == b) {
        return a;
    }
    
    // Create a uniform distribution between [a, b]
    std::uniform_real_distribution<double> distribution(a, b);

    return distribution(gen);
}

// Checks if a graph represented by adjacency lists is connected using BFS
bool is_connected(const std::vector<std::unordered_set<int> >  &adj)
{
    int n = adj.size();
    if (n == 0)
        return true;
    std::vector<bool> visited(n, false);
    std::queue<int> q;
    q.push(0);
    visited[0] = true;
    int count = 1;
    while (!q.empty())
    {
        int u = q.front();
        q.pop();
        for (int v : adj[u])
        {
            if (!visited[v])
            {
                visited[v] = true;
                count++;
                q.push(v);
            }
        }
    }
    return count == n;
}

// Generates a connected graph with n nodes, where each node has degree between 3 and 6
// Uses configuration model with degree sequence modification
std::vector<std::vector<minerID_t> > generate_graph(int n) {
    std::vector<std::vector<minerID_t> > adj(n);

    if (n < 4) {
        std::cerr << "Error: n must be at least 4." << std::endl;
        exit(1);
    }

    if (n == 4) {
        // Return K4
        for (int i = 0; i < 4; ++i) {
            for (int j = 0; j < 4; ++j) {
                if (i != j)
                    adj[i].push_back(j);
            }
        }
        return adj;
    }

    std::vector<int> degrees(n, 3);
    if (n % 2 != 0)
        degrees[0] = 4; // For odd n, set one node to degree 4

    bool connected = false;
    while (!connected) {
        std::vector<minerID_t> stubs;
        for (int i = 0; i < n; ++i) {
            for (int j = 0; j < degrees[i]; ++j) {
                stubs.push_back(i);
            }
        }

        shuffle(stubs.begin(), stubs.end(), gen);

        std::vector<std::vector<minerID_t> > adj(n);
        std::unordered_set<std::pair<minerID_t, minerID_t>, pair_hash> edges;
        bool valid = true;

        for (size_t i = 0; i < stubs.size(); i += 2) {
            minerID_t u = stubs[i];
            minerID_t v = stubs[i + 1];
            if (u == v) {
                valid = false;
                break;
            }
            // Ensure u < v to avoid duplicates
            if (u > v)
                std::swap(u, v);
            if (edges.find(std::make_pair(u, v)) != edges.end()) {
                valid = false;
                break;
            }
            edges.insert(std::make_pair(u, v));
            adj[u].push_back(v);
            adj[v].push_back(u);
        }

        if (!valid)
            continue;

        // Check connectivity using BFS
        std::vector<bool> visited(n, false);
        std::queue<minerID_t> q;
        q.push(0);
        visited[0] = true;
        int count = 0;

        while (!q.empty()) {
            minerID_t curr = q.front();
            q.pop();
            ++count;

            for (minerID_t neighbor : adj[curr]) {
                if (!visited[neighbor]) {
                    visited[neighbor] = true;
                    q.push(neighbor);
                }
            }
        }

        if (count == n)
            connected = true;
    }

    // Augmentation phase: add edges until degrees reach 6
    std::vector<std::pair<minerID_t, minerID_t> > possible_edges;
    for (minerID_t u = 0; u < n; ++u) {
        for (minerID_t v = u + 1; v < n; ++v) {
            bool exists = false;
            for (minerID_t neighbor : adj[u]) {
                if (neighbor == v) {
                    exists = true;
                    break;
                }
            }
            if (!exists)
                possible_edges.emplace_back(u, v);
        }
    }

    // shuffle(possible_edges.begin(), possible_edges.end(), std::mt19937(std::random_device()()));

    shuffle(possible_edges.begin(), possible_edges.end(), gen);

    std::vector<minerID_t> current_degrees(n);
    for (minerID_t i = 0; i < n; ++i)
        current_degrees[i] = adj[i].size();

    for (const auto& edge : possible_edges) {
        minerID_t u = edge.first;
        minerID_t v = edge.second;
        if (current_degrees[u] < 6 && current_degrees[v] < 6) {
            adj[u].push_back(v);
            adj[v].push_back(u);
            ++current_degrees[u];
            ++current_degrees[v];
        }
    }

    return adj;
}

std::vector<std::vector<minerID_t>> permute_graph(std::vector<std::vector<minerID_t>> adj, int totalNodes){
    std::vector<int> permutation = getRandomPermutation(totalNodes);
    std::vector<std::vector<minerID_t>> permuted_adj(totalNodes);
    for (int i = 0; i < totalNodes; ++i) {
        for (int neighbor : adj[i]) {
            permuted_adj[permutation[i]].push_back(permutation[neighbor]);
        }
    }
    return permuted_adj;
}

// Generates network topology with latency and bandwidth parameters
// z0 represents the fraction of slow miners in the network
// Returns a matrix of pairs (latency, bandwidth) for each connection
std::vector<std::vector<std::pair<int, int>>> generateNetworkTopology(int totalNodes, int totalMaliciousNodes){
    std::vector<std::vector<minerID_t>> adj = permute_graph(generate_graph(totalNodes), totalNodes);
    std::vector<std::vector<minerID_t>> adj_matrix(totalNodes, std::vector<minerID_t>(totalNodes, -1));
    for(int i = 0; i < totalNodes; i++){
        for(int j = 0; j < adj[i].size(); j++){
            adj_matrix[i][adj[i][j]] = adj[i][j];
        }
    }
    std::vector<std::vector<std::pair<int, int>>> networkTopology(totalNodes);
    for(int i = 0; i < totalNodes; i++){
        networkTopology[i] = std::vector<std::pair<int, int> > (adj_matrix[i].size());
    }

    maliciousMiners.clear();
    honestMiners.clear();

    for(int i = 0; i < totalMaliciousNodes; i++) maliciousMiners.insert(i);
    for(int i = 0; i < totalNodes; i++) honestMiners.insert(i);

    for(int i = 0; i < totalNodes; i++){
        for(int j = 0; j < totalNodes; j++){
            if(adj_matrix[i][j] == -1){
                networkTopology[i][j] = std::make_pair(-1, -1);
                continue;
            }
            if(honestMiners.count(i) > 0 || honestMiners.count(j) > 0){
                networkTopology[i][j] = std::make_pair(getUniformRandom(10, 500), (5*Mb));
            }
            else{
                networkTopology[i][j] = std::make_pair(getUniformRandom(10, 500), (100*Mb));
            }
        }
    }
    return networkTopology;
}

std::vector<std::vector<std::pair<int, int>>> generateMaliciousNetworkTopology(int totalMaliciousNodes){
    std::vector<std::vector<minerID_t>> adj = generate_graph(totalMaliciousNodes);
    std::vector<std::vector<minerID_t>> adj_matrix(totalMaliciousNodes, std::vector<minerID_t>(totalMaliciousNodes, -1));
    for(int i = 0; i < totalMaliciousNodes; i++){
        for(int j = 0; j < adj[i].size(); j++){
            adj_matrix[i][adj[i][j]] = adj[i][j];
        }
    }
    std::vector<std::vector<std::pair<int, int> > > networkTopology(totalMaliciousNodes);
    for(int i = 0; i < totalMaliciousNodes; i++){
        networkTopology[i] = std::vector<std::pair<int, int> > (adj_matrix[i].size());
    }

    for(int i = 0; i < totalMaliciousNodes; i++){
        for(int j = 0; j < totalMaliciousNodes; j++){
            if(adj_matrix[i][j] == -1){
                networkTopology[i][j] = std::make_pair(-1, -1);
                continue;
            }
            if(honestMiners.count(i) > 0 || honestMiners.count(j) > 0){
                networkTopology[i][j] = std::make_pair(getUniformRandom(1, 10), (5*Mb));
            }
            else{
                networkTopology[i][j] = std::make_pair(getUniformRandom(1, 10), (100*Mb));
            }
        }
    }
    return networkTopology;
}

// Returns a set of high CPU miners
// z1 represents the fraction of low CPU miners
// (1-z1) represents the fraction of high CPU miners

// Not needed for Part2
// std::set<minerID_t> getHighCPUMiners(int n, float z1){
//     std::set<minerID_t> highCPUMiners;
//     std::vector<int> perm = getRandomPermutation(n);
//     for(int i = 0; i < (1-z1) * n; i++){
//         highCPUMiners.insert(perm[i]);
//     }
//     return highCPUMiners;   
// }