#include "miner.hpp"
#include "def.hpp"
#include "utils.hpp"

// Network topology representing connections between miners
extern std::vector<std::vector<std::pair<int, int> > > networkTopology;
extern int TIMEOUT;

// Constructor: initializes a miner with given ID, network parameters and genesis block
Miner::Miner(minerID_t id, int totalMiners, int txnInterval, int blkInterval, Block genesisBlock, std::vector<minerID_t> neighbors)
    : id(id), blockTree(id, genesisBlock), totalMiners(totalMiners), 
      txnInterval(txnInterval), blkInterval(blkInterval), 
      processingTxnTime(-1), processingBlockID(-1), totalBlocksGenerated(0), neighbors(neighbors) {}

// Copy constructor: creates a deep copy of another miner instance
Miner::Miner(const Miner& other)
    : id(other.id), blockTree(BlockTree(other.blockTree)), 
      totalMiners(other.totalMiners), txnInterval(other.txnInterval), 
      blkInterval(other.blkInterval), processingTxnTime(other.processingTxnTime), 
      processingBlockID(other.processingBlockID), 
      totalBlocksGenerated(other.totalBlocksGenerated) {}

// Assignment operator: performs deep copy assignment between miner instances
Miner& Miner::operator=(const Miner& other) {
    this->id = other.id;
    this->blockTree = BlockTree(other.blockTree);
    this->totalMiners = other.totalMiners;
    this->txnInterval = other.txnInterval;
    this->blkInterval = other.blkInterval;
    this->processingTxnTime = other.processingTxnTime;
    this->processingBlockID = other.processingBlockID;
    this->totalBlocksGenerated = other.totalBlocksGenerated;
    return *this;
}

// Move constructor: transfers ownership of resources from another miner instance
Miner::Miner(const Miner&& other)
    : id(other.id), blockTree(std::move(other.blockTree)), 
      totalMiners(other.totalMiners), txnInterval(other.txnInterval), 
      blkInterval(other.blkInterval), processingTxnTime(other.processingTxnTime), 
      processingBlockID(other.processingBlockID) {}

// Move assignment operator: transfers ownership of resources between miner instances
Miner& Miner::operator=(const Miner&& other) {
    this->id = other.id;
    this->blockTree = std::move(other.blockTree);
    this->totalMiners = other.totalMiners;
    this->txnInterval = other.txnInterval;
    this->blkInterval = other.blkInterval;
    this->processingTxnTime = other.processingTxnTime;
    this->processingBlockID = other.processingBlockID;
    this->totalBlocksGenerated = other.totalBlocksGenerated;
    return *this;
}

// Equality comparison operator: compares miners based on their IDs
bool Miner::operator==(const Miner& other) {
    return this->id == other.id;
}

// Returns the unique identifier of this miner
int Miner::getID() const {
    return id;
}

// Generates and returns both transaction and block events for the current timestamp
std::vector<Event*> Miner::getEvents(time_t currentTime) {
    std::vector<Event*> events;
    std::vector<Event*> transactions = genTransaction(currentTime);
    std::vector<Event*> blocks = genBlock(currentTime);
    std::vector<Event*> gets = genGetRequest(currentTime);
    events.insert(events.end(), transactions.begin(), transactions.end());
    events.insert(events.end(), blocks.begin(), blocks.end());
    events.insert(events.end(), gets.begin(), gets.end());
    return events;
}

// Generates a new transaction if:
// 1. Not currently processing another transaction
// 2. Has sufficient balance
// 3. Randomly selects receiver and amount
// Returns a broadcast event for the new transaction
std::vector<Event*> Miner::genTransaction(time_t currentTime, bool malicious) {
    // Don't generate if already processing a transaction
    if (processingTxnTime > currentTime) {
        return std::vector<Event*>();
    }
    
    // Check miner's balance
    int balance = blockTree.getBalance();
    if (balance <= 0) {
        return std::vector<Event*>();
    }

    // Generate random transaction amount and receiver
    double transactionAmount = getUniformRandom(0, balance);
    if (transactionAmount == 0) {
        return std::vector<Event*>();
    }
    
    // Select random receiver (different from self)
    minerID_t receiver = getUniformRandom(1, totalMiners);
    while(receiver == id) {
        receiver = getUniformRandom(1, totalMiners);
    }
    minerID_t sender = id;
    // Schedule transaction with exponential delay
    double expDelay = getExponentialRandom(txnInterval); 
    time_t scheduledTxnTime = currentTime + static_cast<time_t>(expDelay);
    processingTxnID = Counter::getTxnID();
    processingTxnTime = scheduledTxnTime;
    
    // Create and return transaction broadcast event
    Transaction* transaction = new Transaction(processingTxnID, TransactionType::NORMAL, 
                                            id, receiver, transactionAmount);
    std::vector<Event*> tmpEvent{new TransactionEvent(EventType::SEND_TRANSACTION, *transaction, 
                 scheduledTxnTime, id, id, minerID_t(-1), true)};
    if(malicious) tmpEvent.push_back(new TransactionEvent(EventType::SEND_TRANSACTION, *transaction, 
                 scheduledTxnTime, id, id, minerID_t(-1), true, true));
}

hash_t Miner::genHash(Block block) {
    hash_t hash_blk = block.hash();
    return hash_blk;
}

std::vector<Event*> Miner::genGetRequest(time_t currentTime) {
    std::vector<Event*> events;
    for(auto [hash, timeout_time] : timeout) {
        if (timeout_time < currentTime) {
            minerID_t neighbor = blockHashToMiners[hash].front();
            blockHashToMiners[hash].pop();
            timeout[hash] = currentTime + TIMEOUT;
            events.push_back(new GetEvent(EventType::SEND_GET, hash, currentTime, id, id, neighbor));
        }
    }
    return events;
}

// Generates a new block if not currently processing one
// 1. Creates block with coinbase transaction (mining reward)
// 2. Selects valid transactions from mempool
// 3. Validates block before finalizing
// Returns a block creation event
std::vector<Event*> Miner::genBlock(time_t currentTime) {
    if (processingBlockID > 0) {
        return std::vector<Event*>();
    }

    // Calculate block creation delay and prepare block parameters
    double expDelay = getExponentialRandom(blkInterval);
    Block parent = blockTree.getCurrent();
    blockID_t parentID = parent.id;
    int height = parent.height + 1;
    blockID_t blockID = Counter::getBlockID();
    processingBlockID = blockID;
    time_t scheduledBlkTime = currentTime + static_cast<time_t>(expDelay);

    // Select transactions from mempool
    std::vector<Transaction> transactions(memPool.begin(), memPool.end());
    int totalTxn = transactions.size();
    std::vector<Transaction> selectedTxn;

    // Create coinbase transaction (mining reward)
    Transaction coinbase(Counter::getTxnID(), TransactionType::COINBASE, 
                        id, id, COINBASE_REWARD);
    
    // Create new block with coinbase transaction
    Block* block = new Block(blockID, height, parentID, scheduledBlkTime, id);
    block->transactions.push_back(coinbase);

    // Add valid transactions from mempool
    if (totalTxn > 0) {
        int txnCount = getUniformRandom(0, std::min(Kb-1, totalTxn));
        selectedTxn = std::vector<Transaction>(txnCount);
        std::map<minerID_t, int> txnCountMap;
        for(int i = 0; i < transactions.size() && block->transactions.size() < totalTxn; i++){
            if (txnCountMap.find(transactions[i].sender) == txnCountMap.end()){
                txnCountMap[transactions[i].sender] = 0;
            }
            int value = transactions[i].amount;
            if (txnCountMap[transactions[i].sender] + value > blockTree.getBalance(transactions[i].sender)){
                continue;
            }
            txnCountMap[transactions[i].sender] += value;
            block->transactions.push_back(transactions[i]);
            memPool.erase(transactions[i]);
        }
        if (!blockTree.validateBlock(*block)){
            while(block->transactions.size() > 1){
                memPool.insert(block->transactions.back());
                block->transactions.pop_back();
            }
        }
    }
    hash_t hash_blk = genHash(*block);
    blockHashToID[hash_blk] = blockID;
    return {new HashEvent(EventType::SEND_HASH, hash_blk, scheduledBlkTime, id, id, minerID_t(-1), true)};
}

// Processes received transactions:
// 1. Checks for duplicates
// 2. Adds to mempool
// 3. Propagates to neighboring miners
// Returns events for transaction propagation
std::vector<Event*> Miner::receiveTransactions(TransactionEvent event, bool malicious) {
    // Avoid processing duplicate transactions
    if (!this->txnToMiner[event.transaction.id].empty()) {
        return std::vector<Event*>();
    }

    // Add to mempool and prepare for propagation
    memPool.insert(event.transaction);
    std::vector<minerID_t> neighbors = getNeighbors();
    txnToMiner[event.transaction.id].insert(event.owner);

    // Create events to propagate transaction to neighbors
    std::vector<Event*> newEvents;
    for (auto peer: neighbors) {
        if(txnToMiner[event.transaction.id].find(peer) == 
           txnToMiner[event.transaction.id].end()) {
            txnToMiner[event.transaction.id].insert(peer);
            newEvents.push_back(new TransactionEvent(EventType::SEND_TRANSACTION, 
                                    event.transaction, event.timestamp, event.owner, id, peer, true));
            if(malicious) {
                newEvents.push_back(new TransactionEvent(EventType::SEND_TRANSACTION, 
                                    event.transaction, event.timestamp, event.owner, id, peer, true, true));
            }
        }
    }
    return newEvents;
}

// Processes received blocks:
// 1. Checks for duplicates
// 2. Adds block to tree
// 3. Handles chain reorganization if necessary
// 4. Propagates block to neighbors
// Returns events for block propagation
std::vector<Event*> Miner::receiveBlock(BlockEvent event, bool malicious) {
    if ( ! this->blkToMiner[event.block.id].empty() ) {
        return std::vector<Event*> ();
    }
    hash_t hash_blk = event.block.hash();

    if(blockHashToMiners.find(hash_blk) == blockHashToMiners.end()){
        return std::vector<Event*>();
    }
    if(blockTree.addBlock(event.block, event.timestamp) < 0){
        return std::vector<Event*>();
    }
    gotBlock[hash_blk] = true;
    timeout.erase(hash_blk);
    blockHashToMiners.erase(hash_blk);
    blockHashToID[hash_blk] = event.block.id;

    std::vector<Event*> newEvents;

    bool chainSwitched = blockTree.switchToLongestChain((event.block), memPool);

    Block possibleAddedChild;

    do {
        possibleAddedChild = blockTree.addCachedChild();
        if ( possibleAddedChild.id >= 0 ) {
            chainSwitched |= blockTree.switchToLongestChain(possibleAddedChild, memPool);
            std::vector<minerID_t> neighbors = getNeighbors();
            for (auto peer: neighbors){
                if(blkToMiner[possibleAddedChild.id].find(peer) == blkToMiner[possibleAddedChild.id].end()){
                    blkToMiner[possibleAddedChild.id].insert(peer);
                    newEvents.push_back(new HashEvent(EventType::SEND_HASH, possibleAddedChild.hash(), event.timestamp, id, id, peer, false, event.malicious));
                    if(malicious && !event.malicious) {
                        newEvents.push_back(new HashEvent(EventType::SEND_HASH, possibleAddedChild.hash(), event.timestamp, id, id, peer, true, true));
                    }
                }
            }
        }
    } while ( possibleAddedChild.id >= 0 );

    if ( chainSwitched ) {
        processingBlockID = -1;
        std::vector<Event*> genBlocks = this->genBlock(event.timestamp);
        newEvents.insert(newEvents.end(), genBlocks.begin(), genBlocks.end());
    }
    
    blkToMiner[event.block.id].insert(event.owner);
    std::vector<minerID_t> neighbors = getNeighbors();

    for (auto peer: neighbors){
        if(blkToMiner[event.block.id].find(peer) == blkToMiner[event.block.id].end()){
            blkToMiner[event.block.id].insert(peer);
            newEvents.push_back(new HashEvent(EventType::SEND_HASH, event.block.hash(), event.timestamp, event.owner, id, peer, false, event.malicious));
            if(malicious && !event.malicious) {
                newEvents.push_back(new HashEvent(EventType::SEND_HASH, event.block.hash(), event.timestamp, event.owner, id, peer, true, true));
            }
        }
    }

    return newEvents;
}

std::vector<Event*> Miner::receiveHash(HashEvent event) {
    if (gotBlock[event.hash]){
        return std::vector<Event*>();
    }
    blockHashToMiners[event.hash].push(event.sender);
    if(timeout[event.hash] < event.timestamp){
        minerID_t neighbor = blockHashToMiners[event.hash].front();
        blockHashToMiners[event.hash].pop();
        timeout[event.hash] = event.timestamp + TIMEOUT;
        return {new GetEvent(EventType::SEND_GET, event.hash, event.timestamp, id, id, neighbor, false, event.malicious)};
    }
    return std::vector<Event*>();
}

std::vector<Event*> Miner::receiveGet(GetEvent event) {
    if (!gotBlock[event.hash]){
        return std::vector<Event*>();
    }
    blockID_t blockID = blockHashToID[event.hash];
    Block block = blockTree.getBlock(blockID);
    return {new BlockEvent(EventType::SEND_BLOCK, block, event.timestamp, id, id, event.sender, true, event.malicious)};
}

// Returns list of miners directly connected to this miner in the network
std::vector<minerID_t> Miner::getNeighbors() {
    return neighbors;
}

// Confirms block creation:
// Returns true if the block matches currently processing block
// Updates tree and resets processing state
bool Miner::confirmBlock(BlockEvent event) {
    if(event.block.id == processingBlockID) {
        this->totalBlocksGenerated++;
        blockTree.addBlock(event.block, event.timestamp);
        blockTree.switchToLongestChain(event.block, memPool);
        processingBlockID = -1;
        return true;
    }
    return false;
}

// Returns ratio of blocks in longest chain to total blocks generated
float Miner::getRatio() {
    return blockTree.getRatio(this->totalBlocksGenerated);
}

// Prints debug information about miner's current state
void Miner::printMiner(){
    std::cout << "Miner ID: " << id << std::endl;
    std::cout << "Processing Block ID: " << processingBlockID << std::endl;
    std::cout << "Processing Transaction ID: " << processingTxnID << std::endl;
    std::cout << "Processing Block Time: " << processingBlockTime << std::endl;
    std::cout << "Processing Transaction Time: " << processingTxnTime << std::endl;
    std::cout << "Balance: " << blockTree.getBalance() << std::endl;
    std::cout << "Block Interval: " << blkInterval << std::endl;
    std::cout << "Transaction Interval: " << txnInterval << std::endl;
}

// Destructor: exports block tree to DOT format for visualization
Miner::~Miner(){
    blockTree.exportToDot("blockTree-" + std::to_string(id) + ".dot");
}

// Prints summary statistics about the miner's block tree
void Miner::printSummary(bool fast, bool high) {
    this->blockTree.printSummary(fast, high, this->totalBlocksGenerated);
}

// Returns average length of branches in the block tree
float Miner::getAvgBranchLength() {
    return blockTree.averageBranchLength();
}

std::vector<Event*> MaliciousMiner::receiveBlock(BlockEvent event, bool malicious) {
    return Miner::receiveBlock(event, true);
}

std::vector<Event*> MaliciousMiner::receiveGet(GetEvent event) {
   if(!event.malicious && eclipse) {
    return std::vector<Event*>();
   }
   return Miner::receiveGet(event);
}

std::vector<Event*> MaliciousMiner::genTransaction(time_t timestamp, bool malicious) {
    return Miner::genTransaction(timestamp, true);
}

std::vector<Event*> MaliciousMiner::receiveBroadcastPrivateChain(BroadcastPrivateChainEvent event) {
    std::vector<Event*> newEvents;
    if(receivedBroadcastPrivateChain.find(event.block_id) == receivedBroadcastPrivateChain.end()){
        receivedBroadcastPrivateChain[event.block_id] = true;
        for(auto peer: maliciousNeighbors) {
            newEvents.push_back(new BroadcastPrivateChainEvent(EventType::BROADCAST_PRIVATE_CHAIN, event.block_id, event.timestamp, event.owner, id, peer, true, true));
        }
        std::vector<hash_t> hashes;
        Block block = blockTree.getBlock(event.block_id);
        while(block.id != 0) {
            hashes.push_back(block.hash());
            block = blockTree.getNextBlock(block.id);
        }
        for(auto hash: hashes) {
            for(auto peer: neighbors) {
                newEvents.push_back(new HashEvent(EventType::SEND_HASH, hash, event.timestamp, event.owner, id, peer, true, true));
            }
        }
    }
    return newEvents;
}

std::vector<Event*> MaliciousMiner::receiveTransactions(TransactionEvent event, bool malicious) {
    return Miner::receiveTransactions(event, true);
}