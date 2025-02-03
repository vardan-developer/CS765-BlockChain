#include "miner.hpp"

Miner::Miner(minerID_t id): id(id), blockTree(id) {}

bool Miner::operator==(const Miner& other) const{
    return this->id == other.id;
}

std::vector<Event> Miner::getEvents() const{
    return this->events;
}

Transaction Miner::genTransaction(){
    
}

Block Miner::genBlock(){

}

std::vector<Event> Miner::receiveTransactions(Event event){

}

std::vector<Event> Miner::receiveBlock(Event event){

}

std::vector<minerID_t> Miner::getNeighbors() const{

}