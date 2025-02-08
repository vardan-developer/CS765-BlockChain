#ifndef MINER_HPP
#define MINER_HPP

#include "def.hpp"
#include "blockTree.hpp"
#include "transaction.hpp"
#include "event.hpp"


class Miner{
private:
    minerID_t id;
    std::set<Transaction> memPool;
    std::vector<Event> events;
    BlockTree blockTree;
    blockID_t processingBlockID;
    txnID_t processingTxnID;
    std::map<txnID_t, std::set<minerID_t>> txnToMiner;
    std::map<blockID_t, std::set<minerID_t>> blkToMiner;
    int totalMiners;
    int txnInterval;
    int blkInterval;
    time_t processingTxnTime;
    time_t processingBlockTime;
    int totalBlocksGenerated;
    
public:
    Miner(minerID_t id, int totalMiners, int txnInterval, int blkInterval, Block genesisBlock);
    bool operator==(const Miner& other) ;
    Miner(const Miner&& other);
    Miner& operator=(const Miner&& other);
    Miner(const Miner& other);
    Miner& operator=(const Miner& other);
    ~Miner();
    std::vector<Event> getEvents(time_t currentTime) ;
    std::vector<Event> genTransaction(time_t currentTime);
    std::vector<Event> genBlock(time_t currentTime);
    std::vector<Event> receiveTransactions(Event event);
    std::vector<Event> receiveBlock(Event event);
    std::vector<minerID_t> getNeighbors() ;
    bool confirmBlock(Event event);
    int getID() const;
    void printMiner();
    void printSummary(bool fast, bool high);
};


#endif