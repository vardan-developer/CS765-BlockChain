#include "sim.hpp"

extern std::vector<std::vector<minerID_t> > networkTopology;

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
    switch(event.type){
        case EventType::SEND_BROADCAST_TRANSACTION:
            miners[event.owner].receiveTransactions(event);
            break;
        case EventType::SEND_BROADCAST_BLOCK:
            miners[event.owner].receiveBlock(event);
            break;
        case EventType::RECEIVE_BROADCAST_TRANSACTION:
            miners[event.owner].receiveTransactions(event);
            break;
        case EventType::RECEIVE_BROADCAST_BLOCK:
            miners[event.owner].receiveBlock(event);
            break;
        case EventType::BLOCK_CREATION:
            miners[event.owner].genBlock(currentTime);
            break;
        case EventType::BROADCAST_BLOCK:
            
            break;
        case EventType::BROADCAST_TRANSACTION:
            miners[event.owner].broadcastTransaction(event);
            break;
    }
}
