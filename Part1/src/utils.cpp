#include "utils.hpp"


// Add these definitions before the Counter methods
blockID_t Counter::blockIDCount = 1;
txnID_t Counter::txnIDCount = 1;

blockID_t Counter::getBlockID(){
    return blockIDCount++;
}

txnID_t Counter::getTxnID(){
    return txnIDCount++;
}


double getExponentialRandom(double mean) {
    if (mean <= 0) {
        throw std::invalid_argument("Mean must be greater than zero.");
    }

    // Create a random device and a random number generator
    std::random_device rd;
    std::mt19937 gen(rd());  

    // Create an exponential distribution with lambda = 1/mean
    std::exponential_distribution<double> distribution(1.0 / mean);

    return distribution(gen);
}

double getUniformRandom(double a, double b) {
    if (a > b) {
        throw std::invalid_argument("Lower bound must be less than upper bound.");
    }
    if (a == b) {
        return a;
    }

    // Create a random device and a random number generator
    std::random_device rd;
    std::mt19937 gen(rd());  

    // Create a uniform distribution between [a, b]
    std::uniform_real_distribution<double> distribution(a, b);

    return distribution(gen);
}

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

        shuffle(stubs.begin(), stubs.end(), std::mt19937(std::random_device()()));

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

    shuffle(possible_edges.begin(), possible_edges.end(), std::mt19937(std::random_device()()));

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

std::vector<std::vector<std::pair<minerID_t, std::pair<int, int> > > > generateNetworkTopology(int n, float z0){
    std::vector<std::vector<minerID_t> > adj = generate_graph(n);
    std::vector<std::vector<std::pair<minerID_t, std::pair<int, int> > > > networkTopology(n);
    for(int i = 0; i < n; i++){
        networkTopology[i] = std::vector<std::pair<minerID_t, std::pair<int, int> > >(adj[i].size());
    }
    std::set<minerID_t> fastMiners;
    std::set<minerID_t> slowMiners;
    for(int i = 0; i < n; i++){
        if(getUniformRandom(0, 1) < z0){
            fastMiners.insert(i);
        }
        else{
            slowMiners.insert(i);
        }
    }

    for(int i = 0; i < n; i++){
        for(int j = 0; j < adj[i].size(); j++){
            minerID_t u = i;
            minerID_t v = adj[i][j];
            if(slowMiners.count(u) > 0 || slowMiners.count(v) > 0){
                networkTopology[i][j] = std::make_pair(v, std::make_pair(getUniformRandom(10, 500), (5*1024)));
            }
            else{
                networkTopology[i][j] = std::make_pair(v, std::make_pair(getUniformRandom(10, 500), (100*1024)));
            }
        }
    }
    return networkTopology;
}

std::set<minerID_t> getHighCPUMiners(int n, float z1){
    std::set<minerID_t> highCPUMiners;
    for(int i = 0; i < n; i++){
        if(getUniformRandom(0, 1) > z1){
            highCPUMiners.insert(i);
        }
    }
    return highCPUMiners;   
}