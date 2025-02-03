#include "sim.hpp"
#include <ctime>
#include "utils.hpp"

extern std::vector<std::vector<std::pair<minerID_t, std::pair<int, int> > > > networkTopology;

Simulator::Simulator(int n, int txnInterval, int blkInterval){
    totalMiners = n;
    txnInterval = txnInterval;
    blkInterval = blkInterval;
    for(int i = 0; i < n; i++){
        miners.push_back(Miner(i, n, txnInterval, blkInterval));
    }
}

void Simulator::run(){
    while(!events.empty()){
        Event event = events.top();
        events.pop();
        processEvent(event);
    }
}

std::vector<Event> Simulator::getEvents(){
    std::vector<Event> events;
    for(int i = 0; i < totalMiners; i++){
        std::vector<Event> minerEvents = miners[i].getEvents(currentTime);
        events.insert(events.end(), minerEvents.begin(), minerEvents.end());
    }
    return events;
}

void Simulator::addEvent(Event event){
    events.push(event);
}

void Simulator::processEvent(Event event){
    currentTime = event.timestamp;
    switch(event.type){
        case EventType::SEND_BROADCAST_TRANSACTION: {
            time_t latency = networkTopology[event.owner][event.receiver].first + getExponentialRandom(96.0/networkTopology[event.owner][event.receiver].second.second) + (event.transaction->dataSize()/1024.0)/networkTopology[event.owner][event.receiver].second.second;
            addEvent(Event(EventType::RECEIVE_BROADCAST_TRANSACTION, event.transaction, currentTime + latency, event.owner, event.receiver));
            break;
        }
        case EventType::SEND_BROADCAST_BLOCK: {
            time_t latency = networkTopology[event.owner][event.receiver].first + getExponentialRandom(96.0/networkTopology[event.owner][event.receiver].second.second) + (event.block->dataSize()/1024.0)/networkTopology[event.owner][event.receiver].second.second;
            addEvent(Event(EventType::RECEIVE_BROADCAST_BLOCK, event.block, currentTime + latency, event.owner, event.receiver));
            break;
        }
        case EventType::RECEIVE_BROADCAST_TRANSACTION:
            miners[event.owner].receiveTransactions(event);
            break;
        case EventType::RECEIVE_BROADCAST_BLOCK:
            miners[event.owner].receiveBlock(event);
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
                time_t latency = neighbor.second.first + getExponentialRandom(96.0/neighbor.second.second) + (event.block->dataSize()/1024.0)/neighbor.second.second;
                Event sendBlockEvent(EventType::RECEIVE_BROADCAST_BLOCK, event.block, currentTime + latency, neighbor.first, -1);
                addEvent(sendBlockEvent);
            }
            break;
        }
        case EventType::BROADCAST_TRANSACTION: {
            for(auto neighbor : networkTopology[event.owner]){
                if(neighbor.first == event.owner) continue;
                time_t latency = neighbor.second.first + getExponentialRandom(96.0/neighbor.second.second) + (event.transaction->dataSize()/1024.0)/neighbor.second.second;
                Event sendTransactionEvent(EventType::RECEIVE_BROADCAST_TRANSACTION, event.transaction, currentTime + latency, neighbor.first, -1);
                addEvent(sendTransactionEvent);
            }
            break;
        }
    }
}
