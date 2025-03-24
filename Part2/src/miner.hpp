#ifndef MINER_HPP
#define MINER_HPP

#include "def.hpp"
#include "blockTree.hpp"
#include "transaction.hpp"
#include "event.hpp"
#include <functional>   
/**
 * @brief Class representing a miner in the blockchain network
 * Handles transaction creation, block mining, and network communication
 */
class Miner{
protected:
    minerID_t id;                  // Unique identifier for this miner
    std::set<Transaction> memPool; // Memory pool of pending transactions
    std::vector<Event*> events;     // Queue of events to be processed
    BlockTree blockTree;           // Local copy of the blockchain
    blockID_t processingBlockID;   // ID of block currently being processed
    txnID_t processingTxnID;      // ID of transaction currently being processed
    
    // Maps to track which miners have received which transactions/blocks
    std::map<txnID_t, std::set<minerID_t>> txnToMiner;
    std::map<blockID_t, std::set<minerID_t>> blkToMiner; 
    std::map<hash_t, bool> gotBlock;            // Whether a block has been received for a given hash
    std::map<hash_t, blockID_t> blockHashToID;  // Map from block hash to block ID
    std::map<hash_t, std::queue<minerID_t>> blockHashToMiners; // Map from block hash to miners who sent the hash to me

    int totalMiners;              // Total number of miners in the network
    int txnInterval;              // Time interval between transaction generations
    int blkInterval;              // Time interval between block generations
    time_t processingTxnTime;     // Timestamp of last transaction processing
    time_t processingBlockTime;   // Timestamp of last block processing
    int totalBlocksGenerated;     // Counter for total blocks created by this miner
    std::vector<minerID_t> neighbors; // Neighbors of this miner
    std::map<hash_t, time_t> timeout;                 // Timeout for sending next get request
    
public:
    // Constructor: Initialize miner with ID and network parameters
    Miner(minerID_t id, int totalMiners, int txnInterval, int blkInterval, Block genesisBlock, std::vector<minerID_t> neighbors);
    bool operator==(const Miner& other) ;
    Miner(const Miner&& other);
    Miner& operator=(const Miner&& other);
    Miner(const Miner& other);
    Miner& operator=(const Miner& other);
    ~Miner();
    std::vector<Event*> getEvents(time_t currentTime) ;
    std::vector<Event*> genTransaction(time_t currentTime, bool malicious = false);
    std::vector<Event*> genBlock(time_t currentTime);
    std::vector<Event*> receiveTransactions(TransactionEvent event, bool malicious = false);
    std::vector<Event*> receiveBlock(BlockEvent event, bool malicious = false);
    std::vector<Event*> receiveHash(HashEvent event);
    std::vector<Event*> receiveGet(GetEvent event);
    hash_t genHash(Block block);
    std::vector<Event*> genGetRequest(time_t currentTime);
    std::vector<minerID_t> getNeighbors() ;
    bool confirmBlock(BlockEvent event);
    int getID() const;
    void printMiner();
    void printSummary(bool fast, bool high);
    float getRatio();
    float getAvgBranchLength();
};

class MaliciousMiner: public Miner {
protected:
    bool eclipse;
    std::vector<minerID_t> maliciousNeighbors;
    std::map<blockID_t, bool> receivedBroadcastPrivateChain;
public:
    MaliciousMiner(minerID_t id, int totalMiners, int txnInterval, int blkInterval, Block genesisBlock, std::vector<minerID_t> neighbors, bool eclipse = false);
    // Constructor: Initialize miner with ID and network parameters
    bool operator==(const MaliciousMiner& other) ;
    MaliciousMiner(const MaliciousMiner&& other);
    MaliciousMiner& operator=(const MaliciousMiner&& other);
    MaliciousMiner(const MaliciousMiner& other);
    MaliciousMiner& operator=(const MaliciousMiner& other);
    ~MaliciousMiner();
    std::vector<Event*> receiveBlock(BlockEvent event, bool malicious = false);
    std::vector<Event*> receiveGet(GetEvent event);
    std::vector<Event*> receiveTransactions(TransactionEvent event, bool malicious = false);
    std::vector<Event*> genTransaction(time_t timestamp, bool malicious=true);
    std::vector<Event*> receiveBroadcastPrivateChain(BroadcastPrivateChainEvent event);
    void printMiner();
    void printSummary(bool fast, bool high);
};

#endif