#include "miner.hpp"
#include "utils.hpp"

extern std::vector<std::vector<std::pair<minerID_t, std::pair<int, int> > > > networkTopology;

Miner::Miner(minerID_t id, int totalMiners, int txnInterval, int blkInterval, Block genesisBlock): id(id), blockTree(id, genesisBlock), totalMiners(totalMiners), txnInterval(txnInterval), blkInterval(blkInterval), processingTxnTime(-1), processingBlockID(-1) {}

Miner::Miner(const Miner& other): id(other.id), blockTree(BlockTree(other.blockTree)), totalMiners(other.totalMiners), txnInterval(other.txnInterval), blkInterval(other.blkInterval), processingTxnTime(other.processingTxnTime), processingBlockID(other.processingBlockID) {}

Miner& Miner::operator=(const Miner& other){
    this->id = other.id;
    this->blockTree = BlockTree(other.blockTree);
    this->totalMiners = other.totalMiners;
    this->txnInterval = other.txnInterval;
    this->blkInterval = other.blkInterval;
    this->processingTxnTime = other.processingTxnTime;
    this->processingBlockID = other.processingBlockID;
    return *this;
}

Miner::Miner(const Miner&& other): id(other.id), blockTree(std::move(other.blockTree)), totalMiners(other.totalMiners), txnInterval(other.txnInterval), blkInterval(other.blkInterval), processingTxnTime(other.processingTxnTime), processingBlockID(other.processingBlockID) {}

Miner& Miner::operator=(const Miner&& other){
    this->id = other.id;
    this->blockTree = std::move(other.blockTree);
    this->totalMiners = other.totalMiners;
    this->txnInterval = other.txnInterval;
    this->blkInterval = other.blkInterval;
    this->processingTxnTime = other.processingTxnTime;
    this->processingBlockID = other.processingBlockID;
    return *this;
}

bool Miner::operator==(const Miner& other){
    return this->id == other.id;
}

int Miner::getID() const {
    return id;
}

std::vector<Event> Miner::getEvents(time_t currentTime){
    std::vector<Event> events;
    std::vector<Event> transactions = genTransaction(currentTime);
    std::vector<Event> blocks = genBlock(currentTime);
    events.insert(events.end(), transactions.begin(), transactions.end());
    events.insert(events.end(), blocks.begin(), blocks.end());
    return events;
}

std::vector<Event> Miner::genTransaction(time_t currentTime){
    if (processingTxnTime > currentTime) {
        return std::vector<Event>();
    }
    int balance = blockTree.getBalance(); // get balance of miner
    if (balance <= 0) {
        return std::vector<Event>();
    }
    double transactionAmount = getUniformRandom(0, balance); // generate random transaction amount
    if (transactionAmount == 0) {
        return std::vector<Event>();
    }
    minerID_t receiver = getUniformRandom(1, totalMiners); // generate random receiver
    while(receiver == id){
        receiver = getUniformRandom(1, totalMiners);
    }
    double expDelay = getExponentialRandom(txnInterval); 
    time_t scheduledTxnTime = currentTime + static_cast<time_t>(expDelay);
    processingTxnID = Counter::getTxnID();
    processingTxnTime = scheduledTxnTime;
    Transaction* transaction = new Transaction(processingTxnID, TransactionType::NORMAL, id, receiver, transactionAmount);
    return {Event(EventType::BROADCAST_TRANSACTION, transaction, scheduledTxnTime, id, -1)}; // create transaction event
}

std::vector<Event> Miner::genBlock(time_t currentTime){
    if (processingBlockID > 0) {
        std::cout << "Miner: " << id << " Rejects Block Creation, Currently Processing Block ID: " << processingBlockID << std::endl;
        return std::vector<Event>();
    }
    double expDelay = getExponentialRandom(blkInterval);
    Block parent = blockTree.getCurrent();
    blockID_t parentID = parent.id;
    int height = parent.height + 1;
    blockID_t blockID = Counter::getBlockID();
    processingBlockID = blockID;
    time_t scheduledBlkTime = currentTime + static_cast<time_t>(expDelay);
    std::vector<Transaction> transactions(memPool.begin(), memPool.end());
    int totalTxn = transactions.size();
    std::vector<Transaction> selectedTxn;

    Transaction coinbase = Transaction(Counter::getTxnID(), TransactionType::COINBASE, id, id, 50);
    
    Block* block = new Block(blockID, height, parentID, scheduledBlkTime); // add coinbase
    block->transactions.push_back(coinbase);
    if (totalTxn > 0){
        int txnCount = getUniformRandom(0, std::min(100, totalTxn));
        selectedTxn = std::vector<Transaction>(txnCount);
        for(int i = 0; i < txnCount; i++){
            block->transactions.push_back(transactions[i]);
            if (!blockTree.validateBlock(*block)){
                block->transactions.pop_back();
            }
            else{
                memPool.erase(transactions[i]);
            }
        }
    }
    std::cout << "Miner: " << id << " Creates Block ID: " << blockID << ", Height: " << height << ", Parent ID: " << parentID << ", Timestamp: " << scheduledBlkTime << std::endl;
    return {Event(EventType::BLOCK_CREATION, block, scheduledBlkTime, id, -1)};
}

std::vector<Event> Miner::receiveTransactions(Event event){

    memPool.insert(*(event.transaction));
    std::vector<minerID_t> neighbors = getNeighbors();
    
    txnToMiner[event.transaction->id].insert(event.owner);

    std::vector<Event> newEvents;

    for (auto peer: neighbors){
        if(txnToMiner[event.transaction->id].find(peer) == txnToMiner[event.transaction->id].end()){
            txnToMiner[event.transaction->id].insert(peer);
            newEvents.push_back(Event(EventType::SEND_BROADCAST_TRANSACTION, event.transaction, event.timestamp, id, peer));
        }
    }

    return newEvents;
}

std::vector<Event> Miner::receiveBlock(Event event){

    // printMiner();


    if(blockTree.addBlock(*(event.block), event.timestamp) < 0){
        // std::cout << "Block: " << event.block->id << ", Height: " << event.block->height << ", Parent ID: " << event.block->parentID << ", Timestamp: " << event.timestamp << std::endl;
        return std::vector<Event>();
    }
    std::cout << "Block: " << event.block->id << ", Height: " << event.block->height << ", Parent ID: " << event.block->parentID << ", Timestamp: " << event.timestamp << std::endl;

    std::vector<Event> newEvents;

    if ( blockTree.switchToLongestChain(*(event.block), memPool) ) {
        processingBlockID = -1;
        std::vector<Event> genBlocks = this->genBlock(event.timestamp);
        newEvents.insert(newEvents.end(), genBlocks.begin(), genBlocks.end());
    }

    else if ( blockTree.getCurrent().id == event.block->id ) {
        processingBlockID = -1;
        std::vector<Event> genBlocks = this->genBlock(event.timestamp);
        newEvents.insert(newEvents.end(), genBlocks.begin(), genBlocks.end());
    }

    blkToMiner[event.block->id].insert(event.owner);
    std::vector<minerID_t> neighbors = getNeighbors();

    for (auto peer: neighbors){
        if(blkToMiner[event.block->id].find(peer) == blkToMiner[event.block->id].end()){
            blkToMiner[event.block->id].insert(peer);
            newEvents.push_back(Event(EventType::SEND_BROADCAST_BLOCK, event.block, event.timestamp, id, peer));
        }
    }

    return newEvents;
}

std::vector<minerID_t> Miner::getNeighbors() {
    std::vector<minerID_t> neighbors;
    for(auto neighbor: networkTopology[id]){
        if ( neighbor.first >= 0 ) neighbors.push_back(neighbor.first);
    }
    return neighbors;
}

bool Miner::confirmBlock(Event event) {
    // std::cout << "Confirming Block: " << event.block->id << std::endl;
    if(event.block->id == processingBlockID) {
        blockTree.addBlock(*(event.block), event.timestamp);
        blockTree.switchToLongestChain(*(event.block), memPool);
        processingBlockID = -1;
        return true;
    }
    return false;
}

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

Miner::~Miner(){
    if ( id == 1 ) blockTree.exportToDot("blockTree.dot");
}