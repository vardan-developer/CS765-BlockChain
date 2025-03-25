#ifndef SIM_HPP
#define SIM_HPP

#include "block.hpp"
#include "event.hpp"
#include "miner.hpp"
#include "network.hpp"
#include "parser.hpp"
#include <ctime>

std::string eventTypeToString(EventType type);

struct EventComparator {
    bool operator()(Event* a, Event* b) const {
        return *a > *b; // Uses Event's `operator>`
    }
};

class Simulator{
private:
    int totalMiners;
    int txnInterval;
    int blkInterval;
    double maliciousFraction;
    double Tt;
    time_t timeLimit;
    long long blkCount;
    bool show;
    time_t currentTime;
    Network *honestNetwork, *maliciousNetwork;
    std::vector<Miner *> miners;
    std::priority_queue<Event*, std::vector<Event*>, EventComparator> events;
    std::set<blockID_t> blockSet;
    Block createGenesisBlock();
    void generateGraphViz(const std::string& honestNetworkFile = "honestGraph.dot", const std::string& maliciousNetworkFile = "maliciousGraph.dot");
    int eventsProcessed;
    
public:
    // Simulator(int n, int txnInterval, int blkInterval, time_t timeLimit = 10 * 60 * 1000, long long blkCount = 100);
    Simulator(ProgramSettings & settings);
    ~Simulator();
    void run();
    void getEvents();
    void addEvent(Event * event);
    void addEvents(std::vector<Event*> events);
    void processEvent(Event * event);

    void processSendHashEvent(HashEvent * event);
    void processReceiveHashEvent(HashEvent * event);
    void processSendBlockEvent(BlockEvent * event);
    void processReceiveBlockEvent(BlockEvent * event);
    void processSendTransactionEvent(TransactionEvent * event);
    void processReceiveTransactionEvent(TransactionEvent * event);
    void processSendGetEvent(GetEvent * event);
    void processReceiveGetEvent(GetEvent * event);
    void processBroadcastPrivateChain(BroadcastPrivateChainEvent* event);
    void processBlockCreation(HashEvent* event);
};

#endif
