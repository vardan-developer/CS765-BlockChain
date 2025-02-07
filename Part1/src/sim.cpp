#include "sim.hpp"
#include <ctime>
#include "def.hpp"
#include "utils.hpp"

extern std::vector<std::vector<std::pair<int, int> > > networkTopology;
extern std::set<minerID_t> highCPUMiners;
extern std::set<minerID_t> fastMiners;
extern std::set<minerID_t> slowMiners;

void Simulator::generateGraphViz(const std::string& filename) {
    std::ofstream file(filename);
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
    file << "    node [shape=circle, style=filled, width=0.3, height=0.3];\n";
    file << "    l1 [label=\"Fast/High\", fillcolor=\"green\"];\n";
    file << "    l2 [label=\"Fast/Low\", fillcolor=\"red\"];\n";
    file << "    l3 [label=\"Slow/High\", fillcolor=\"lightblue\"];\n";
    file << "    l4 [label=\"Slow/Low\", fillcolor=\"lightgray\"];\n";
    file << "  }\n\n";

    int numMiners = networkTopology.size();

    for (int i = 0; i < numMiners; i++) {
        std::string color = (fastMiners.count(i)) ? ( highCPUMiners.count(i) ? "green" : "red") : ( highCPUMiners.count(i) ? "lightblue" : "lightgray");

        file << "  " << i << " [label=\"" << i << "\", fillcolor=\"" << color << "\"];\n";
    }

    for (int i = 0; i < numMiners; i++) {
        for (int j = i + 1; j < numMiners; j++) { 
            if (networkTopology[i][j].first >= 0) {
                std::string edgeLabel = (slowMiners.count(i) && slowMiners.count(j)) ? "Slow" : "Fast";
            }
        }
    }

    file << "}\n";
    file.close();
    std::cout << "Graph saved to " << filename << std::endl;
}

Simulator::Simulator(int n, int txnInterval, int blkInterval, time_t timeLimit, long long blkCount){ // received in ms
    totalMiners = n;
    txnInterval = txnInterval;
    blkInterval = blkInterval; // this is I for each miner it will be I/(fraction of hashing power)
    int totalHashingPower = 0;
    for(int i = 0; i < n; i++){
        totalHashingPower += (highCPUMiners.count(i) > 0) ? 10 : 1;
    }
    currentTime = 0;
    Block genesisBlock = createGenesisBlock();
    miners.reserve(n);
    for(int i = 0; i < n; i++){
        Miner * new_miner = new Miner(i, n, txnInterval, blkInterval*(totalHashingPower/(highCPUMiners.count(i) > 0 ? 10 : 1)), genesisBlock);
        miners.push_back(new_miner);
    }
    std::cout << n << " Miners initialized\n";
    this->timeLimit = timeLimit;
    this->blkCount = blkCount;
}

Simulator::~Simulator(){
    this->generateGraphViz();
    for ( Miner * miner : miners) {
        bool fast = false;
        for ( minerID_t miner_t = 0; miner_t < totalMiners; miner_t++) {
            fast |= ( networkTopology[miner->getID()][miner_t].second == 100 * Mb );
        }
        miner->printSummary(fast, highCPUMiners.count(miner->getID()));
        delete miner;
    }
}

void Simulator::run(){
    do {
        getEvents();
        if (events.empty()) continue;
        Event event = events.top();
        events.pop();
        processEvent(event);
        if ( this->currentTime > this->timeLimit) break;
        // std::cout << this->currentTime << std::endl;
        
    } while( this->blkCount );
}

void Simulator::getEvents(){
    for(int i = 0; i < totalMiners; i++){
        std::vector<Event> minerEvents = miners[i]->getEvents(currentTime);
        while (minerEvents.size() > 0) {
            events.push( minerEvents.back() );
            minerEvents.pop_back();
        }
    }
}

void Simulator::addEvent(Event event){
    events.push(event);
}

void Simulator::addEvents(std::vector<Event> events){
    for(auto event : events){
        this->events.push(event);
    }
}

std::string eventTypeToString(EventType type){
    switch(type){
        case EventType::SEND_BROADCAST_TRANSACTION: return "SEND_BROADCAST_TRANSACTION";
        case EventType::SEND_BROADCAST_BLOCK: return "SEND_BROADCAST_BLOCK";
        case EventType::RECEIVE_BROADCAST_TRANSACTION: return "RECEIVE_BROADCAST_TRANSACTION";
        case EventType::RECEIVE_BROADCAST_BLOCK: return "RECEIVE_BROADCAST_BLOCK";
        case EventType::BLOCK_CREATION: return "BLOCK_CREATION";
        case EventType::BROADCAST_BLOCK: return "BROADCAST_BLOCK";
        case EventType::BROADCAST_TRANSACTION: return "BROADCAST_TRANSACTION";
        default: return "UNKNOWN";
    }
}

void Simulator::processEvent(Event event){
    currentTime = event.timestamp;
    switch(event.type){
        case EventType::SEND_BROADCAST_TRANSACTION: {
            time_t latency = networkTopology[event.owner][event.receiver].first + ceil(getExponentialRandom(96.0 * Kb * 1000/networkTopology[event.owner][event.receiver].second)) + ceil((Kb * 8.0 * 1000)/networkTopology[event.owner][event.receiver].second);
            addEvent(Event(EventType::RECEIVE_BROADCAST_TRANSACTION, event.transaction, currentTime + latency, event.owner, event.receiver));
            break;
        }
        case EventType::SEND_BROADCAST_BLOCK: {
            time_t latency = networkTopology[event.owner][event.receiver].first + ceil(getExponentialRandom(96.0 * Kb * 1000/networkTopology[event.owner][event.receiver].second)) + ceil((event.block->dataSize() * 1000.0)/networkTopology[event.owner][event.receiver].second);
            addEvent(Event(EventType::RECEIVE_BROADCAST_BLOCK, event.block, currentTime + latency, event.owner, event.receiver));
            break;
        }
        case EventType::RECEIVE_BROADCAST_TRANSACTION:
            addEvents(miners[event.receiver]->receiveTransactions(event));
            break;
        case EventType::RECEIVE_BROADCAST_BLOCK:
            addEvents(miners[event.receiver]->receiveBlock(event));
            break;
        case EventType::BLOCK_CREATION: {
            if ( miners[event.owner]->confirmBlock(event) ) {
                addEvent(Event(EventType::BROADCAST_BLOCK, event.block, currentTime, event.owner, 0));
                this->blkCount--;
                std::cout << "Block " << event.block->id << " Confirmed by Miner" << event.block->owner << '\n';
            }
            break;
        }
        case EventType::BROADCAST_BLOCK: {
            auto neighbors = miners[event.owner]->getNeighbors();
            for(auto neighbor : neighbors){
                if(neighbor == event.owner) continue;
                Event sendBlockEvent(EventType::SEND_BROADCAST_BLOCK, event.block, currentTime, event.owner, neighbor);
                addEvent(sendBlockEvent);
            }
            break;
        }
        case EventType::BROADCAST_TRANSACTION: {
            auto neighbors = miners[event.owner]->getNeighbors();
            for(auto neighbor : neighbors){
                if(neighbor == event.owner) continue;
                Event sendTransactionEvent(EventType::SEND_BROADCAST_TRANSACTION, event.transaction, currentTime, event.owner, neighbor);
                addEvent(sendTransactionEvent);
            }
            break;
        }
    }
}

Block Simulator::createGenesisBlock(){
    Block genesisBlock(0, 0, 0, 0, -1);
    return genesisBlock;
}