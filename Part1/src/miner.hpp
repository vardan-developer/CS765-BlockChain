#ifndef MINER_HPP
#define MINER_HPP

#include "def.hpp"
#include "blockTree.hpp"
#include "transaction.hpp"
#include "event.hpp"

/**
 * @brief Class representing a miner in the blockchain network
 * Handles transaction creation, block mining, and network communication
 */
class Miner{
private:
    minerID_t id;                  // Unique identifier for this miner
    std::set<Transaction> memPool; // Memory pool of pending transactions
    std::vector<Event> events;     // Queue of events to be processed
    BlockTree blockTree;           // Local copy of the blockchain
    blockID_t processingBlockID;   // ID of block currently being processed
    txnID_t processingTxnID;      // ID of transaction currently being processed
    
    // Maps to track which miners have received which transactions/blocks
    std::map<txnID_t, std::set<minerID_t>> txnToMiner;
    std::map<blockID_t, std::set<minerID_t>> blkToMiner;
    
    int totalMiners;              // Total number of miners in the network
    int txnInterval;              // Time interval between transaction generations
    int blkInterval;              // Time interval between block generations
    time_t processingTxnTime;     // Timestamp of last transaction processing
    time_t processingBlockTime;   // Timestamp of last block processing
    int totalBlocksGenerated;     // Counter for total blocks created by this miner
    
public:
    // Constructor: Initialize miner with ID and network parameters
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
    float getRatio();
    float getAvgBranchLength();
};


#endif