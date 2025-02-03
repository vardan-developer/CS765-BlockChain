#ifndef MINER_HPP
#define MINER_HPP

#include "defs.hpp"
#include "blockTree.hpp"

class Miner{
private:
    minerID_t id;
    std::vector<Transaction> memPool;
    std::vector<Event> events;
    BlockTree blockTree;

public:
    Miner(minerID_t id);
    bool operator==(const Miner& other) const;
    std::vector<Event> getEvents() const;
    Transaction genTransaction();
    Block genBlock();
    std::vector<Event> receiveTransactions(Event event);
    std::vector<Event> receiveBlock(Event event);
    std::vector<minerID_t> getNeighbors() const;
};


#endif