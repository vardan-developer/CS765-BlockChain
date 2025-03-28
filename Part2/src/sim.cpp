#include "sim.hpp"
#include <cerrno>
#include <climits>
#include <ctime>
#include <fstream>
#include <iostream>
#include <ostream>
#include <vector>
#include "block.hpp"
#include "def.hpp"
#include "event.hpp"
#include "miner.hpp"
#include "parser.hpp"
#include "transaction.hpp"

// extern std::vector<std::vector<std::pair<int, int> > > networkTopology;
// extern std::vector<std::vector<std::pair<int, int> > > networkTopologyMalicious;
// extern std::set<minerID_t> highCPUMiners;
extern std::set<minerID_t> maliciousMiners;
extern std::set<minerID_t> honestMiners;
time_t TIMEOUT;

// Generates a GraphViz visualization of the network topology
// Creates a graph showing miners and their connections, with colors indicating miner types
void Simulator::generateGraphViz(const std::string& honestNetworkFile, const std::string& maliciousNetworkFile) {
    std::ofstream file(honestNetworkFile);
    if (!file.is_open()) {
        std::cerr << "Error opening file!" << std::endl;
        return;
    }

    file << "graph Network {\n";
    file << "  node [shape=circle, style=filled, fontname=\"Arial\", width=0.5, height=0.5];\n";

    // Add legend
    file << "  subgraph cluster_legend {\n";
    file << "    label=\"Legend\";\n";
    file << "    style=filled;\n";
    file << "    color=lightgrey;\n";
    file << "    node [shape=circle, style=filled, width=0.05, height=0.05, fontsize=8, margin=0.0];\n";
    file << "    l2 [label=\"Malicious\", fillcolor=\"red\"];\n";
    file << "    l3 [label=\"Honest\", fillcolor=\"lightblue\"];\n";
    file << "  }\n\n";

    for (int i = 0; i < totalMiners; i++) {
        std::string color = (maliciousMiners.count(i)) ? "red" : "lightblue";
        file << "  " << i << " [label=\"" << i << "\", fillcolor=\"" << color << "\"];\n";
    }

    for (int i = 0; i < totalMiners; i++) {
        for (int j = i + 1; j < totalMiners; j++) { 
            if (honestNetwork->networkTopology[i][j].first >= 0) {
                file << "  " << i << " -- " << j << " [color=blue];\n";
            }
        }
    }

    file << "}\n";
    file.close();

    file = std::ofstream(maliciousNetworkFile);
    if (!file.is_open()) {
        std::cerr << "Error opening file!" << std::endl;
        return;
    }

    file << "graph Network {\n";
    file << "  node [shape=circle, style=filled, fontname=\"Arial\", width=0.5, height=0.5];\n";

    // Add legend
    file << "  subgraph cluster_legend {\n";
    file << "    label=\"Legend\";\n";
    file << "    style=filled;\n";
    file << "    color=lightgrey;\n";
    file << "    node [shape=circle, style=filled, width=0.05, height=0.05, fontsize=8, margin=0.0];\n";
    file << "  }\n\n";

    int totalMaliciousNodes = maliciousNetwork->networkTopology.size();

    for (int i = 0; i < totalMaliciousNodes; i++) {
        std::string color = "red";
        file << "  " << i << " [label=\"" << i << "\", fillcolor=\"" << color << "\"];\n";
    }

    for (int i = 0; i < totalMaliciousNodes; i++) {
        for (int j = i + 1; j < totalMaliciousNodes; j++) { 
            if (maliciousNetwork->networkTopology[i][j].first >= 0) {
                file << "  " << i << " -- " << j << " [color=blue];\n";
            }
        }
    }

    file << "}\n";
    file.close();
}

// Constructor: Initializes the simulator with network parameters
// n: number of miners
// txnInterval: time between transactions
// blkInterval: base block interval
// timeLimit: simulation time limit
// blkCount: number of blocks to simulate

// Simulator::Simulator(int n, int txnInterval, int blkInterval, time_t timeLimit, long long blkCount) {
//     totalMiners = n;
//     txnInterval = txnInterval;
//     blkInterval = blkInterval; // this is I for each miner it will be I/(fraction of hashing power)
//     int totalHashingPower = 0;
//     for(int i = 0; i < n; i++){
//         totalHashingPower += (highCPUMiners.count(i) > 0) ? 10 : 1;
//     }
//     currentTime = 0;
//     Block genesisBlock = createGenesisBlock();
//     miners.reserve(n);
//     for(int i = 0; i < n; i++){
//         Miner * new_miner = new Miner(i, n, txnInterval, blkInterval*(totalHashingPower/(highCPUMiners.count(i) > 0 ? 10 : 1)), genesisBlock);
//         miners.push_back(new_miner);
//     }
//     this->timeLimit = timeLimit;
//     this->blkCount = blkCount;
// }

Simulator::Simulator(ProgramSettings & settings):
    totalMiners(settings.totalNodes),
    txnInterval(settings.Ttx),
    blkInterval(settings.I),
    maliciousFraction(settings.malicious),
    Tt(settings.Tt),
    timeLimit(settings.timeLimit),
    blkCount(settings.blkLimit),
    currentTime(0),
    eventsProcessed(0),
    do_gen_block(true)
{
    TIMEOUT = settings.Tt;
    honestNetwork = new Network(totalMiners, maliciousFraction, false);
    maliciousNetwork = new Network(totalMiners, maliciousFraction, true);
    Block genesisBlock = createGenesisBlock();
    miners.reserve(totalMiners);
    Miner* ringMaster = new RingMaster(0, totalMiners, txnInterval, blkInterval * totalMiners / (totalMiners * maliciousFraction), genesisBlock, honestNetwork->getNeighbors(0), maliciousNetwork->getNeighbors(0), settings.eclipse);
    miners.push_back(ringMaster);
    int maliciousCount = totalMiners * maliciousFraction;
    for(int i = 1; i < maliciousCount; i++) {

        Miner * new_miner = new MaliciousMiner(i, totalMiners, txnInterval, INT_MAX, genesisBlock, honestNetwork->getNeighbors(i), maliciousNetwork->getNeighbors(i), settings.eclipse);
        miners.push_back(new_miner);
    }
    for(int i = maliciousCount; i < totalMiners; i++) {
        Miner * new_miner = new Miner(i, totalMiners, txnInterval, blkInterval * totalMiners, genesisBlock, honestNetwork->getNeighbors(i));
        miners.push_back(new_miner);
    }
}

// Destructor: Cleans up resources and prints final statistics
// Calculates and displays average mining ratios for different miner categories
Simulator::~Simulator() {
    this->generateGraphViz();
    std::vector<float> fast_high, fast_low, slow_high, slow_low;
    float tempRatio;
    for ( Miner * miner : miners) {
        // bool fast = maliciousMiners.count(miner->getID());
        // bool high = highCPUMiners.count(miner->getID());
        bool isMalicious = maliciousMiners.count(miner->getID());
        miner->printSummary(isMalicious, true);
        // miner->printSummary(fast, high);
        // if(tempRatio == -1) {delete miner; continue;}
        // if(fast && high) fast_high.push_back(tempRatio);
        // else if(fast && !high) fast_low.push_back(tempRatio);
        // else if(!fast && high) slow_high.push_back(tempRatio);
        // else slow_low.push_back(tempRatio);
        delete miner;
    }
    delete honestNetwork;
    delete maliciousNetwork;
}

// Main simulation loop
// Processes events until either time limit is reached or required blocks are mined
void Simulator::run() {
    do {
        getEvents();
        if (events.empty()) continue;
        Event * event = events.top();
        events.pop();
        processEvent(event);
        delete event;
        if (this->currentTime > this->timeLimit) break;
        if(this->blockSet.size() > this->blkCount) break;
    } while ( this->blockSet.size() <= this->blkCount );
    auto ringMaster = (RingMaster*) miners[0];
    auto newEvents = ringMaster->checkAndBroadcastPrivate(currentTime, true);
    for(auto event : newEvents) {
        this->events.push(event);
    }
    std::cout << "Simulator::run - Reached Here" << std::endl;
    do_gen_block = false;
    while(!events.empty()) {
        getEvents();
        Event * event = events.top();
        events.pop();
        processEvent(event);
        delete event;
    }
}

// Collects pending events from all miners and adds them to the event queue
void Simulator::getEvents() {
    for(int i = 0; i < totalMiners; i++){
        std::vector<Event*> minerEvents = miners[i]->getEvents(currentTime, do_gen_block);
        while (minerEvents.size() > 0) {
            events.push( minerEvents.back() );
            minerEvents.pop_back();
        }
    }
}

// Adds a single event to the simulation event queue
void Simulator::addEvent(Event * event) {
    this->events.push(event);
}

// Adds multiple events to the simulation event queue
void Simulator::addEvents(std::vector<Event*> events) {
    for(auto event : events){
        this->events.push(event);
    }
}

// Utility function to convert EventType enum to string for debugging
std::string eventTypeToString(EventType type) {
    switch(type){
        case EventType::SEND_TRANSACTION: return "SEND_TRANSACTION";
        case EventType::RECEIVE_TRANSACTION: return "RECEIVE_TRANSACTION";
        case EventType::SEND_GET: return "SEND_GET";
        case EventType::RECEIVE_GET: return "RECEIVE_GET";
        case EventType::SEND_BLOCK: return "SEND_BLOCK";
        case EventType::RECEIVE_BLOCK: return "RECEIVE_BLOCK";
        case EventType::SEND_HASH: return "SEND_HASH";
        case EventType::RECEIVE_HASH: return "RECEIVE_HASH";
        case EventType::BROADCAST_PRIVATE_CHAIN: return "BROADCAST_PRIVATE_CHAIN";
        case EventType::BLOCK_CREATION: return "BLOCK_CREATION";
        default: return "UNKNOWN";
    }
}

void Simulator::processSendHashEvent(HashEvent * event) {
    time_t latency;
    if (event->receiver < 0) {
        std::vector<minerID_t> neighbors = event->malicious ? maliciousNetwork->getNeighbors(event->sender) : honestNetwork->getNeighbors(event->sender);
        for(auto neighbor : neighbors){
            if(neighbor == event->sender) continue;
            latency = event->malicious ? maliciousNetwork->getLatency(event->sender, neighbor) : honestNetwork->getLatency(event->sender, neighbor);
            HashEvent* newEvent = new HashEvent(EventType::RECEIVE_HASH, event->hash, event->timestamp + latency, event->owner, event->sender, neighbor, event->broadcast, event->malicious);
            this->events.push((Event*) newEvent);
        }
    } else {
        latency = event->malicious ? maliciousNetwork->getLatency(event->sender, event->receiver) : honestNetwork->getLatency(event->sender, event->receiver);
        HashEvent* newEvent = new HashEvent(EventType::RECEIVE_HASH, event->hash, event->timestamp + latency, event->owner, event->sender, event->receiver, event->broadcast, event->malicious);
        this->events.push((Event*) newEvent);
    }
}

void Simulator::processSendBlockEvent(BlockEvent * event) {
    time_t latency;
    this->blockSet.insert(event->block.id);
    if (event->receiver < 0) {
        std::vector<minerID_t> neighbors = event->malicious ? maliciousNetwork->getNeighbors(event->sender) : honestNetwork->getNeighbors(event->sender);
        for(auto neighbor : neighbors){
            if(neighbor == event->sender) continue;
            latency = event->malicious ? maliciousNetwork->getLatency(event->sender, neighbor) : honestNetwork->getLatency(event->sender, neighbor);
            BlockEvent* newEvent = new BlockEvent(EventType::RECEIVE_BLOCK, event->block, event->timestamp + latency, event->owner, event->sender, neighbor, event->broadcast, event->malicious);
            this->events.push((Event*) newEvent);
        }
    } else {
        latency = event->malicious ? maliciousNetwork->getLatency(event->sender, event->receiver) : honestNetwork->getLatency(event->sender, event->receiver);
        BlockEvent* newEvent = new BlockEvent(EventType::RECEIVE_BLOCK, event->block, event->timestamp + latency, event->owner, event->sender, event->receiver, event->broadcast, event->malicious);
        this->events.push((Event*) newEvent);
    }
}

void Simulator::processSendTransactionEvent(TransactionEvent * event) {
    time_t latency;
    if (event->receiver < 0) {
        std::vector<minerID_t> neighbors = event->malicious ? maliciousNetwork->getNeighbors(event->sender) : honestNetwork->getNeighbors(event->sender);
        for(auto neighbor : neighbors){
            if(neighbor == event->sender) continue;
            latency = event->malicious ? maliciousNetwork->getLatency(event->sender, neighbor) : honestNetwork->getLatency(event->sender, neighbor);
            TransactionEvent* newEvent = new TransactionEvent(EventType::RECEIVE_TRANSACTION, event->transaction, event->timestamp + latency, event->owner, event->sender, neighbor, event->broadcast, event->malicious);
            this->events.push((Event*) newEvent);
        }
    } else {
        latency = event->malicious ? maliciousNetwork->getLatency(event->sender, event->receiver) : honestNetwork->getLatency(event->sender, event->receiver);
        TransactionEvent* newEvent = new TransactionEvent(EventType::RECEIVE_TRANSACTION, event->transaction, event->timestamp + latency, event->owner, event->sender, event->receiver, event->broadcast, event->malicious);
        this->events.push((Event*) newEvent);
    }
}

void Simulator::processSendGetEvent(GetEvent * event) {
    time_t latency;
    if (event->receiver < 0) {
        std::vector<minerID_t> neighbors = event->malicious ? maliciousNetwork->getNeighbors(event->sender) : honestNetwork->getNeighbors(event->sender);
        for(auto neighbor : neighbors){
            if(neighbor == event->sender) continue;
            latency = event->malicious ? maliciousNetwork->getLatency(event->sender, neighbor) : honestNetwork->getLatency(event->sender, neighbor);
            GetEvent* newEvent = new GetEvent(EventType::RECEIVE_GET, event->hash, event->timestamp + latency, event->owner, event->sender, neighbor, event->broadcast, event->malicious);
            this->events.push((Event*) newEvent);
        }
    } else {
        latency = event->malicious ? maliciousNetwork->getLatency(event->sender, event->receiver) : honestNetwork->getLatency(event->sender, event->receiver);
        GetEvent* newEvent = new GetEvent(EventType::RECEIVE_GET, event->hash, event->timestamp + latency, event->owner, event->sender, event->receiver, event->broadcast, event->malicious);
        this->events.push((Event*) newEvent);
    }
}

inline void Simulator::processReceiveHashEvent(HashEvent * event) {
    this->addEvents(this->miners[event->receiver]->receiveHash(* event));
}

inline void Simulator::processReceiveBlockEvent(BlockEvent * event) {

    this->addEvents(this->miners[event->receiver]->receiveBlock(* event, false, do_gen_block));
}

inline void Simulator::processReceiveTransactionEvent(TransactionEvent * event) {
    this->addEvents(this->miners[event->receiver]->receiveTransactions(* event));
}

inline void Simulator::processReceiveGetEvent(GetEvent * event) {
    auto newEvents = this->miners[event->receiver]->receiveGet(* event);
    this->addEvents(newEvents);
}

void Simulator::processBroadcastPrivateChain(BroadcastPrivateChainEvent* event) {
    time_t latency;
    if (event->receiver < 0) {
        std::vector<minerID_t> neighbors = maliciousNetwork->getNeighbors(event->sender);
        for(auto neighbor : neighbors){
            if(neighbor == event->sender) continue;
            latency = maliciousNetwork->getLatency(event->sender, neighbor);
            BroadcastPrivateChainEvent* newEvent = new BroadcastPrivateChainEvent(EventType::BROADCAST_PRIVATE_CHAIN, event->block_id, event->timestamp + latency, event->owner, event->sender, neighbor, event->broadcast, true);
            this->events.push((Event*) newEvent);
        }
    } else {
        ((MaliciousMiner*)(miners[event->receiver]))->receiveBroadcastPrivateChain(* event);
    }
}

void Simulator::processBlockCreation(HashEvent* event) {
    if(miners[event->owner]->confirmBlock(*event)){        
        std::vector<minerID_t> neighbors = event->malicious ? maliciousNetwork->getNeighbors(event->sender) : honestNetwork->getNeighbors(event->sender);
        time_t latency;
        // TODO: Below loop is iterating over wrong neighbors
        for(auto neighbor : neighbors){
            if(neighbor == event->sender) continue;
            latency = event->malicious ? maliciousNetwork->getLatency(event->sender, neighbor) : honestNetwork->getLatency(event->sender, neighbor);
            HashEvent* newEvent = new HashEvent(EventType::SEND_HASH, event->hash,  event->timestamp + latency, event->owner, event->sender, neighbor, event->broadcast, event->malicious);
            this->events.push((Event*) newEvent);
        }
    }
}

// Core event processing function
// Handles different types of events and their propagation through the network:
// - Transaction broadcasting
// - Block broadcasting
// - Block creation
// - Network message propagation
void Simulator::processEvent(Event * event) {
    currentTime = event->timestamp;
    switch(event->type){
        case EventType::SEND_HASH:
            processSendHashEvent((HashEvent*) event);
            break;
        case EventType::RECEIVE_HASH:
            processReceiveHashEvent((HashEvent*) event);
            break;
        case EventType::SEND_BLOCK:
            processSendBlockEvent((BlockEvent*) event);
            break;
        case EventType::RECEIVE_BLOCK:
            processReceiveBlockEvent((BlockEvent*) event);
            break;
        case EventType::SEND_TRANSACTION:
            processSendTransactionEvent((TransactionEvent*) event);
            break;
        case EventType::RECEIVE_TRANSACTION:
            processReceiveTransactionEvent((TransactionEvent*) event);
            break;
        case EventType::SEND_GET:
            processSendGetEvent((GetEvent*) event);
            break;
        case EventType::RECEIVE_GET:
            processReceiveGetEvent((GetEvent*) event);
            break;
        case EventType::BROADCAST_PRIVATE_CHAIN:
            processBroadcastPrivateChain((BroadcastPrivateChainEvent*) event);
            break;
        case EventType::BLOCK_CREATION:
            processBlockCreation((HashEvent*) event);
            break;
        default:
            std::cerr << "UNKNOWN EVENT ENCOUNTERED" << std::endl;
    }
}

// Creates the initial (genesis) block for the blockchain
Block Simulator::createGenesisBlock() {
    Block genesisBlock(0, 0, 0, 0, -1);
    return genesisBlock;
}