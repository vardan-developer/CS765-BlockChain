#include "sim.hpp"
#include <ctime>
#include "utils.hpp"

extern std::vector<std::vector<std::pair<minerID_t, std::pair<int, int> > > > networkTopology;
extern std::set<minerID_t> highCPUMiners;

Simulator::Simulator(int n, int txnInterval, int blkInterval, int limit){ // received in ms
    totalMiners = n;
    txnInterval = txnInterval;
    blkInterval = blkInterval; // this is I for each miner it will be I/(fraction of hashing power)
    int totalHashingPower = 0;
    for(int i = 0; i < n; i++){
        totalHashingPower += (highCPUMiners.count(i) > 0) ? 10 : 1;
    }
    Block genesisBlock = createGenesisBlock();
    miners.reserve(n);
    for(int i = 0; i < n; i++){

        Miner * new_miner = new Miner(i, n, txnInterval, blkInterval*(totalHashingPower/(highCPUMiners.count(i) > 0 ? 10 : 1)), genesisBlock);

        miners.push_back(std::move(* new_miner));

    }
    std::cout << "Miners initialized" << std::endl;
    currentTime = 0;
    this->limit = limit;
}

void Simulator::run(){
    do {
        getEvents();
        if (events.empty()) continue;
        Event event = events.top();
        events.pop();
        processEvent(event);
        limit--;
        if(limit == 0){
            break;
        }
    } while(!events.empty());
}

void Simulator::getEvents(){
    // std::vector<Event> events;
    for(int i = 0; i < totalMiners; i++){
        std::vector<Event> minerEvents = miners[i].getEvents(currentTime);
        // events.insert(events.end(), minerEvents.begin(), minerEvents.end());
        while (minerEvents.size() > 0) {
            // if (minerEvents.back().type == EventType::BLOCK_CREATION) std::cout << "Event: BLOCK_CREATION, Miner " << minerEvents.back().owner << std::endl;
            events.push( minerEvents.back() );
            minerEvents.pop_back();
        }
    }
    // return events;
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
    // std::cout << "Processing event: " << eventTypeToString(event.type) << ", Miner:  " << event.owner << std::endl;
    currentTime = event.timestamp;
    switch(event.type){
        case EventType::SEND_BROADCAST_TRANSACTION: {
            time_t latency = networkTopology[event.owner][event.receiver].second.first + ceil(getExponentialRandom(96000.0/networkTopology[event.owner][event.receiver].second.second)) + ceil((event.transaction->dataSize()*1000/1024.0)/networkTopology[event.owner][event.receiver].second.second);
            addEvent(Event(EventType::RECEIVE_BROADCAST_TRANSACTION, event.transaction, currentTime + latency, event.owner, event.receiver));
            break;
        }
        case EventType::SEND_BROADCAST_BLOCK: {
            time_t latency = networkTopology[event.owner][event.receiver].second.first + ceil(getExponentialRandom(96000.0/networkTopology[event.owner][event.receiver].second.second)) + ceil((event.block->dataSize()*1000/1024.0)/networkTopology[event.owner][event.receiver].second.second);
            addEvent(Event(EventType::RECEIVE_BROADCAST_BLOCK, event.block, currentTime + latency, event.owner, event.receiver));
            break;
        }
        case EventType::RECEIVE_BROADCAST_TRANSACTION:
            addEvents(miners[event.receiver].receiveTransactions(event));
            break;
        case EventType::RECEIVE_BROADCAST_BLOCK:
            addEvents(miners[event.receiver].receiveBlock(event));
            break;
        case EventType::BLOCK_CREATION: {
            if ( miners[event.owner].confirmBlock(event) ) {
                addEvent(Event(EventType::BROADCAST_BLOCK, event.block, currentTime, event.owner, -1));
            }
            break;
        }
        case EventType::BROADCAST_BLOCK: {
            for(auto neighbor : networkTopology[event.owner]){
                if(neighbor.first == event.owner) continue;
                // time_t latency = neighbor.second.first + ceil(getExponentialRandom(96000.0/neighbor.second.second)) + ceil((event.block->dataSize()*1000/1024.0)/neighbor.second.second);
                Event sendBlockEvent(EventType::SEND_BROADCAST_BLOCK, event.block, currentTime, event.owner, neighbor.first);
                addEvent(sendBlockEvent);
            }
            break;
        }
        case EventType::BROADCAST_TRANSACTION: {
            for(auto neighbor : networkTopology[event.owner]){
                if(neighbor.first == event.owner) continue;
                // time_t latency = neighbor.second.first + ceil(getExponentialRandom(96000.0/neighbor.second.second)) + ceil((event.transaction->dataSize()*1000/1024.0)/neighbor.second.second);
                Event sendTransactionEvent(EventType::SEND_BROADCAST_TRANSACTION, event.transaction, currentTime, event.owner, neighbor.first);
                addEvent(sendTransactionEvent);
            }
            break;
        }
    }
}

Block Simulator::createGenesisBlock(){
    Block genesisBlock(0, 0, 0,  currentTime);
    return genesisBlock;
}