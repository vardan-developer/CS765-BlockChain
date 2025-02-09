#include "blockTree.hpp"
#include "def.hpp"
#include "transaction.hpp"
#include <map>
#include <queue>

// Default constructor for tree node
BlockTreeNode::BlockTreeNode(): arrivalTime(0), height(0), parent(nullptr) {}

// Constructor with block data, height, and arrival time
BlockTreeNode::BlockTreeNode(Block block, int height, time_t arrivalTime)
    : block(Block(block)), height(height), arrivalTime(arrivalTime), parent(nullptr) {}

// Constructor with just block data
BlockTreeNode::BlockTreeNode(const Block & block)
    : block(Block(block)), parent(nullptr), height(0), arrivalTime(0) {}

// Copy constructor for tree node
BlockTreeNode::BlockTreeNode(const BlockTreeNode & other)
    : block(Block(other.block)), parent(other.parent), children(other.children), 
      height(other.height), arrivalTime(other.arrivalTime) {}

// Assignment operator for tree node
BlockTreeNode & BlockTreeNode::operator=(const BlockTreeNode & other) {
    this->block = other.block;
    this->parent = other.parent;
    this->children = other.children;
    this->height = other.height;
    this->arrivalTime = other.arrivalTime;
    return *this;
}

// Default constructor for BlockTree
BlockTree::BlockTree()
    : id(0), genesis(nullptr), current(nullptr), 
      file(std::ofstream ("logs/miner-" + std::to_string(id) + ".logs")) {}

// Constructor with miner ID
BlockTree::BlockTree(minerID_t id)
    : id(id), genesis(nullptr), current(nullptr), 
      file(std::ofstream ("logs/miner-" + std::to_string(id) + ".logs")) {}

// Constructor with miner ID and genesis block
BlockTree::BlockTree(minerID_t id, Block genesisBlock)
    : id(id), genesis(new BlockTreeNode(genesisBlock)), current(genesis), 
      file(std::ofstream ("logs/miner-" + std::to_string(id) + ".logs")) {
    blockToNode[genesis->block.id] = genesis;
}

// Destructor - cleans up all nodes in the tree using BFS
BlockTree::~BlockTree() {
    if (!genesis) {
        return;
    }
    std::queue<BlockTreeNode *> q;
    q.push(genesis);

    while (!q.empty()) {
        BlockTreeNode * node = q.front();
        q.pop();
        for (BlockTreeNode * child : node->children) {
            if (child) {
                q.push(child);
            }
        }
        delete node;
    }
}

// Deep copy constructor
BlockTree::BlockTree(const BlockTree & other) {
    this->id = other.id;
    this->genesis = other.genesis ? 
        new BlockTreeNode(other.genesis->block, other.genesis->height, other.genesis->arrivalTime) 
        : nullptr;
    this->genesis = deepCopy(genesis, other.genesis);
    for ( auto & [minerId, balance] : other.balanceMap ) {
        this->balanceMap[minerId] = balance;
    }
    this->current = this->blockToNode[other.current->block.id];
    this->file = std::ofstream ("logs/miner-" + std::to_string(id) + ".logs");
    this->cachedChildren = other.cachedChildren;
}

BlockTree & BlockTree::operator = (const BlockTree & other) {
    this->id = other.id;
    this->genesis = other.genesis ? new BlockTreeNode(other.genesis->block, other.genesis->height, other.genesis->arrivalTime) : nullptr;
    this->genesis = deepCopy(genesis, other.genesis);
    for ( auto & [minerId, balance] : other.balanceMap ) {
        this->balanceMap[minerId] = balance;
    }
    this->current = this->blockToNode[other.current->block.id];
    this->file = std::ofstream ("logs/miner-" + std::to_string(id) + ".logs");
    this->cachedChildren = other.cachedChildren;
    return *this;
}

BlockTree::BlockTree(BlockTree && other) {
    this->id = other.id;
    this->genesis = other.genesis;
    other.genesis = nullptr;
    this->current = other.current;
    other.current = nullptr;
    this->blockToNode = other.blockToNode;
    other.blockToNode.clear();
    this->balanceMap = other.balanceMap;
    other.balanceMap.clear();
    this->file = std::ofstream ("logs/miner-" + std::to_string(id) + ".logs");
    this->cachedChildren = other.cachedChildren;
}

BlockTree & BlockTree::operator = (BlockTree && other) {
    this->id = other.id;
    this->genesis = other.genesis;
    other.genesis = nullptr;
    this->current = other.current;
    other.current = nullptr;
    this->blockToNode = other.blockToNode;
    other.blockToNode.clear();
    this->balanceMap = other.balanceMap;
    other.balanceMap.clear();
    this->file = std::ofstream ("logs/miner-" + std::to_string(id) + ".logs");
    this->cachedChildren = other.cachedChildren;
    return *this;
}

// Helper function to deepcopy the entire tree
BlockTreeNode * BlockTree::deepCopy(BlockTreeNode * root, BlockTreeNode * other) {
    if (!other || !root) {
        return root;
    }
    this->blockToNode[other->block.id] = root;
    for (BlockTreeNode * child : other->children) {
        if (child) {
            BlockTreeNode * newChild = new BlockTreeNode(child->block, child->height, child->arrivalTime);
            newChild->parent = root;
            newChild = deepCopy(newChild, child);
            root->children.push_back(newChild);
        }
    }
    return root;
}

// Find Lowest Common Ancestor of two nodes
BlockTreeNode * BlockTree::getLca(BlockTreeNode * node1, BlockTreeNode * node2) {
    // First equalize heights
    while (node1 != node2) {
        if (node1->height > node2->height) {
            node1 = node1->parent;
        } else {
            node2 = node2->parent;
        }
    }
    // Then move up together until common ancestor found
    while (node1 != node2) {
        node1 = node1->parent;
        node2 = node2->parent;
    }
    return node1;
}

// Validate transactions in a chain
bool BlockTree::validateChain(BlockTreeNode * node) {
    std::map<minerID_t, int> spendingMap;
    // Calculate total spending per miner in this block
    for (Transaction & txn : node->block.transactions) {
        if (txn.type == TransactionType::COINBASE) continue;
        if (spendingMap.find(txn.sender) == spendingMap.end()) {
            spendingMap[txn.sender] = 0;
        }
        spendingMap[txn.sender] += txn.amount;
    }
    // Verify each miner has sufficient balance
    for (auto & [minerId, spendingAmount] : spendingMap) {
        if (balanceMap[minerId] < spendingAmount) {
            return false;
        }
    }
    return true;
}

// Used to verify if the given block can be inserted in the current blockchain
bool BlockTree::validateBlock(Block & block) {
    BlockTreeNode * node = new BlockTreeNode(block);
    node->parent = this->current;
    node->height = this->current->height + 1;
    bool returnValue = validateChain(node);
    delete node;
    return returnValue;
}

// Used to update the balance on indertion of a new tree node
void BlockTree::updateBalance(BlockTreeNode * node) {
    for (Transaction & txn : node->block.transactions) {
        if (balanceMap.find(txn.sender) == balanceMap.end()) balanceMap[txn.sender] = 0;
        if (balanceMap.find(txn.receiver) == balanceMap.end()) balanceMap[txn.receiver] = 0;
        
        if (txn.type != TransactionType::COINBASE) {
            balanceMap[txn.sender] -= txn.amount;
            balanceMap[txn.receiver] += txn.amount;
        } else {
            balanceMap[txn.receiver] += txn.amount;
        }
    }
}

// Add a new block to the tree
int BlockTree::addBlock(Block & block, time_t arrivalTime) {
    // Check if block already exists
    if (blockToNode.find(block.id) != blockToNode.end()) {
        return -1;
    }
    
    // Check if parent exists
    BlockTreeNode * parent = blockToNode[block.parentID];
    if (!parent) {
        this->cachedChildren.insert(std::make_pair(block, arrivalTime));
        return -1;
    }
    // std::cout << "Miner " << id << " trying to add block " << block.id << '\n';
    BlockTreeNode * node = new BlockTreeNode(block);
    node->parent = parent;
    node->height = parent->height + 1;
    if ( validateChain(node) ) {
        blockToNode[block.id] = node;
        parent->children.push_back(node);
        printBlock(node, arrivalTime);
        node->arrivalTime = arrivalTime;
        return std::max(node->height, this->current->height);
    } else {
        // std::cout << "Block " << node->block.id << " Rejected by Miner " << this->id << '\n';
        delete node;
        return -1;
    }
}

// Used to update balance of all the peers after processing the given transaction list
void BlockTree::processTransaction(std::vector<Transaction> & transactions) {
    for ( Transaction & txn : transactions ) {
        if ( txn.type != TransactionType::COINBASE ) {
            balanceMap[txn.sender] -= txn.amount;
        }
        balanceMap[txn.receiver] += txn.amount;
    }
}

// Used to update balance of all the peers after deprocessing the given transaction list
void BlockTree::deProcessTransactions(std::vector<Transaction> & transactions) {
    for ( Transaction & txn : transactions ) {
        if ( txn.type != TransactionType::COINBASE ) {
            balanceMap[txn.sender] += txn.amount;
        }
        balanceMap[txn.receiver] -= txn.amount;
    }
}

// Used to get the balance of the miner who owns this tree object
int BlockTree::getBalance(minerID_t id) {
    if (id == -1) {
        return balanceMap[id];
    }
    return balanceMap[id];
}

// Gets the latest block of the main chain
Block BlockTree::getCurrent() {
    return this->current->block;
}

// This function updates the balances of each peer and other state variables in case of a branch switch
bool BlockTree::switchToLongestChain(Block & block, std::set<Transaction> & memPool) {
    BlockTreeNode * node = blockToNode[block.id];
    if ( node == nullptr || block.id == this->current->block.id ) {
        return false;
    }
    
    if ( node->parent == this->current ) {
        this->current = node;
        updateBalance(this->current);
        for ( Transaction & txn : this->current->block.transactions ) {
            memPool.erase(txn);
        }
        return true;
    } else if ( node->height > this->current->height ) {
        BlockTreeNode * node1 = node, * node2 = this->current;
        std::set<Transaction> memPoolInsert, memPoolErase;
        while ( node1->height > node2->height ) {
                memPoolErase.insert(node1->block.transactions.begin(), node1->block.transactions.end());
                node1 = node1->parent;
                processTransaction(node1->block.transactions);
        }
        while ( node1 != node2 ) {
            memPoolErase.insert(node1->block.transactions.begin(), node1->block.transactions.end());
            processTransaction(node1->block.transactions);
            node1 = node1->parent;
            
            memPoolInsert.insert(node2->block.transactions.begin(), node2->block.transactions.end());
            node2 = node2->parent;
            deProcessTransactions(node2->block.transactions);
        }

        for ( Transaction txn : memPoolInsert ) {
            memPool.insert(txn);
        }
        for ( Transaction txn : memPoolErase ) {
            memPool.erase(txn);
        }

        this->current = node;
        return true;
    }
    return false;
}

// Helper function to print tree
void BlockTree::printTree(std::string filename) const {
    std::ofstream file(filename);
    if ( ! genesis || ! file.is_open() ) {
        return;
    }
    printSubTree(genesis, file);
    file.close();
}

// Helper function to print tree
void BlockTree::printSubTree(BlockTreeNode* node, std::ofstream & file) const {
    file << "( " << node->block.id << " " << node->arrivalTime << '\n';
    for (BlockTreeNode* child : node->children) {
        printSubTree(child, file);
    }
    file << ")" << '\n';
}

// Helper function to print a chain given it's latest node
void BlockTree::printChain(BlockTreeNode* node) const {
    while (node) {
        std::cout << node->block.id << '\n';
        node = node->parent;
    }
}

// Used to create dot files for creating the blocktree graphs
void BlockTree::exportToDot(const std::string & filename) const {
    std::ofstream file(filename);
    if(!file.is_open()){
        std::cerr << "Error opening file: " << filename << std::endl;
        return;
    }
    file << "digraph BlockchainTree {\n";
    file << " node [shape=block];\n";

    std::function<void(BlockTreeNode*)> traverse = [&](BlockTreeNode* node) {
        if (!node) return;

        file << "    \"" << node->block.id << "\" [label=\"Block " << node->block.id 
             << "\\nHeight: " << node->height 
             << "\\nMiner ID: " << node->block.owner
             << "\\nBlock Creation Time: " << node->block.timestamp
             << "\\nTimestamp: " << node->arrivalTime << "\"];\n";

        for (BlockTreeNode* child : node->children) {
            file << "    \"" << node->block.id << "\" -> \"" << child->block.id << "\";\n";
            traverse(child);
        }
    };

    traverse(genesis); // Start from the genesis block
    file << "}\n";
    file.close();
}

// Used to print block arrival logs
void BlockTree::printBlock(BlockTreeNode* node, time_t arrivalTime) {
    file << "Block ID: " << node->block.id << ", Arrival Time: " << arrivalTime << ", Parent ID: " << node->parent->block.id << '\n';
}

// Used to print summary at the end
void BlockTree::printSummary(bool fast, bool highCpu, long long totalBlocksGenerated) {
    file << "Total Blocks Generated: " << totalBlocksGenerated << '\n';
    file << "Total Blocks in Main Chain: " << getMainChainBlockCount() << '\n';
    file << ( fast ? "Fast, " : "Slow, ") << ( highCpu ? "highCpu" : "lowCpu" ) << '\n';
    file << "Average Branch Length: " << averageBranchLength() << '\n';
}

// Helper functions for analysis
long long BlockTree::getMainChainBlockCount() {
    long long ownBlock = 0;
    if ( ! this->genesis ) ownBlock = 0;
    else {
        BlockTreeNode * node = this->current;
        while ( node->height > 0 ) {
            if ( node->block.owner == id ) ownBlock++;
            node = node->parent;
        }
    }
    return ownBlock;
}

// Helper functions for analysis
float BlockTree::getRatio(long long totalBlocksGenerated) {
    long long ownBlock = 0;
    if ( ! this->genesis ) ownBlock = 0;
    else {
        BlockTreeNode * node = this->current;
        while ( node->height > 0 ) {
            if ( node->block.owner == id ) ownBlock++;
            node = node->parent;
        }
    }
    return totalBlocksGenerated ? (float(ownBlock)/totalBlocksGenerated) : -1;
}

// Function used to try adding blocks encountered earlier (cached blocks) if their parent block is now present in the blockchain
Block BlockTree::addCachedChild() {
    for ( auto [block, arrivalTime] : this->cachedChildren ) {
        if ( addBlock(block, arrivalTime) >= 0 ) {
            this->cachedChildren.erase(std::make_pair(block, arrivalTime));
            return block;
        }
    }
    Block newBlock = Block();
    newBlock.id = -1;
    return newBlock;
}

// Helper functions for analysis
std::unordered_set<BlockTreeNode*> BlockTree::findMainChain() {
    std::unordered_set<BlockTreeNode*> mainChain;
    BlockTreeNode* p = this->current;
    while (p) {
        mainChain.insert(p);
        p = p->parent;
    }
    return mainChain;
}

// Helper functions for analysis
void BlockTree::findLeaves(BlockTreeNode* node, std::unordered_set<BlockTreeNode*>& mainChain, std::vector<BlockTreeNode*>& leaves) {
    if (!node) return;

    if (node->children.empty() && mainChain.find(node) == mainChain.end()) {
        leaves.push_back(node);
    }

    for (BlockTreeNode* child : node->children) {
        findLeaves(child, mainChain, leaves);
    }
}

// Helper functions for analysis
int BlockTree::findDistanceToMainChain(BlockTreeNode* leaf, std::unordered_set<BlockTreeNode*>& mainChain) {
    int distance = 0;
    BlockTreeNode* p = leaf;
    while (mainChain.find(p) == mainChain.end()) {
        distance++;
        p = p->parent;
    }
    return distance;
}

// Helper functions for analysis
float BlockTree::averageBranchLength() {
    std::unordered_set<BlockTreeNode*> mainChain = findMainChain();

    std::vector<BlockTreeNode*> leaves;
    findLeaves(genesis, mainChain, leaves);

    if (leaves.empty()) return 0.0;

    int totalDistance = 0;
    for (BlockTreeNode* leaf : leaves) {
        totalDistance += findDistanceToMainChain(leaf, mainChain);
    }

    return (float)totalDistance / leaves.size();
}