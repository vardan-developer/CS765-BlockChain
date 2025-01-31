#include "utils.hpp"

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
    if (a >= b) {
        throw std::invalid_argument("Lower bound must be less than upper bound.");
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

std::vector<std::unordered_set<int> > generate_graph(int n)
{
    if (n < 5)
    {
        throw std::invalid_argument("n must be at least 5");
    }
    if (n == 5)
    {
        std::vector<std::unordered_set<int> > adj(5);
        for (int i = 0; i < 5; ++i)
        {
            for (int j = 0; j < 5; ++j)
            {
                if (i != j)
                {
                    adj[i].insert(j);
                }
            }
        }
        return adj;
    }

    std::vector<std::unordered_set<int> > adj(n);
    std::random_device rd;
    std::mt19937 rng(rd());

    // Step 1: Create 4-regular connected graph
    bool connected = false;
    while (!connected)
    {
        // Reset adjacency list
        for (auto &s : adj)
        {
            s.clear();
        }

        // Create stubs
        std::vector<int> stubs;
        for (int i = 0; i < n; ++i)
        {
            for (int j = 0; j < 4; ++j)
            {
                stubs.push_back(i);
            }
        }

        // Shuffle stubs
        shuffle(stubs.begin(), stubs.end(), rng);

        // Pair stubs
        for (size_t i = 0; i < stubs.size(); i += 2)
        {
            int u = stubs[i];
            int v = stubs[i + 1];
            if (u != v && adj[u].count(v) == 0)
            {
                adj[u].insert(v);
                adj[v].insert(u);
            }
        }

        // Check connected
        connected = is_connected(adj);
    }

    // Step 2: Add edges to reach degree 5 where possible
    std::vector<std::pair<int, int> > possible_edges;
    for (int u = 0; u < n; ++u)
    {
        for (int v = u + 1; v < n; ++v)
        {
            if (adj[u].count(v) == 0)
            {
                possible_edges.emplace_back(u, v);
            }
        }
    }

    std::shuffle(possible_edges.begin(), possible_edges.end(), rng);

    for (const auto &[u, v] : possible_edges)
    {
        if (adj[u].size() < 5 && adj[v].size() < 5)
        {
            adj[u].insert(v);
            adj[v].insert(u);
        }
    }

    return adj;
}