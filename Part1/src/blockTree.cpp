#include "blockTree.hpp"
#include "def.hpp"
#include "transaction.hpp"
#include <map>
#include <queue>

BlockTreeNode::BlockTreeNode(): arrivalTime(0), height(0), parent(nullptr) {}

BlockTreeNode::BlockTreeNode(Block block, int height, time_t arrivalTime): block(Block(block)), height(height), arrivalTime(arrivalTime), parent(nullptr) {}

BlockTreeNode::BlockTreeNode(const Block & block): block(Block(block)), parent(nullptr), height(0), arrivalTime(0) {}

BlockTreeNode::BlockTreeNode(const BlockTreeNode & other): block(Block(other.block)), parent(other.parent), children(other.children), height(other.height), arrivalTime(other.arrivalTime) {}

BlockTreeNode & BlockTreeNode::operator=(const BlockTreeNode & other) {
    this->block = other.block;
    this->parent = other.parent;
    this->children = other.children;
    this->height = other.height;
    this->arrivalTime = other.arrivalTime;
    return *this;
}

BlockTree::BlockTree(): id(0), genesis(nullptr), current(nullptr), file(std::ofstream ("miner-" + std::to_string(id) + ".logs")) {}

BlockTree::BlockTree(minerID_t id): id(id), genesis(nullptr), current(nullptr), file(std::ofstream ("miner-" + std::to_string(id) + ".logs")) {}

BlockTree::BlockTree(minerID_t id, Block genesisBlock): id(id), genesis(new BlockTreeNode(genesisBlock)), current(genesis), file(std::ofstream ("miner-" + std::to_string(id) + ".logs")) {
    blockToNode[genesis->block.id] = genesis;
}

BlockTree::~BlockTree() {
    if ( ! genesis ) {
        return;
    }
    std::queue<BlockTreeNode *> q;
    q.push(genesis);

    while ( !q.empty() ) {
        BlockTreeNode * node = q.front();
        q.pop();
        for ( BlockTreeNode * child : node->children ) {
            if ( child ) {
                q.push(child);
            }
        }
        delete node;
    }
}

BlockTree::BlockTree(const BlockTree & other) {
    this->id = other.id;
    this->genesis = other.genesis ? new BlockTreeNode(other.genesis->block, other.genesis->height, other.genesis->arrivalTime) : nullptr;
    this->genesis = deepCopy(genesis, other.genesis);
    for ( auto & [minerId, balance] : other.balanceMap ) {
        this->balanceMap[minerId] = balance;
    }
    this->current = this->blockToNode[other.current->block.id];
    this->file = std::ofstream ("miner-" + std::to_string(id) + ".logs");
}

BlockTree & BlockTree::operator = (const BlockTree & other) {
    this->id = other.id;
    this->genesis = other.genesis ? new BlockTreeNode(other.genesis->block, other.genesis->height, other.genesis->arrivalTime) : nullptr;
    this->genesis = deepCopy(genesis, other.genesis);
    for ( auto & [minerId, balance] : other.balanceMap ) {
        this->balanceMap[minerId] = balance;
    }
    this->current = this->blockToNode[other.current->block.id];
    this->file = std::ofstream ("miner-" + std::to_string(id) + ".logs");
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
    this->file = std::ofstream ("miner-" + std::to_string(id) + ".logs");
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
    this->file = std::ofstream ("miner-" + std::to_string(id) + ".logs");
    return *this;
}

BlockTreeNode * BlockTree::deepCopy(BlockTreeNode * root, BlockTreeNode * other) {
    if ( ! other || ! root ) {
        return root;
    }
    this->blockToNode[other->block.id] = root;
    for ( BlockTreeNode * child : other->children ) {
        if ( child ) {
            BlockTreeNode * newChild = new BlockTreeNode(child->block, child->height, child->arrivalTime);
            newChild->parent = root;
            newChild = deepCopy(newChild, child);
            root->children.push_back(newChild);
        }
    }
    return root;
}

BlockTreeNode * BlockTree::getLca(BlockTreeNode * node1, BlockTreeNode * node2) {
    while (node1 != node2) {
        if (node1->height > node2->height) {
            node1 = node1->parent;
        } else {
            node2 = node2->parent;
        }
    }
    while ( node1 != node2 ) {
        node1 = node1->parent;
        node2 = node2->parent;
    }
    return node1;
}

bool BlockTree::validateChain(BlockTreeNode * node) {
    std::map<minerID_t, int> spendingMap;
    for ( Transaction & txn : node->block.transactions) {
        if ( txn.type == TransactionType::COINBASE ) continue;
        if ( spendingMap.find(txn.sender) == spendingMap.end() ) {
            spendingMap[txn.sender] = 0;
        }
        spendingMap[txn.sender] += txn.amount;
    }
    for ( auto & [minerId, spendingAmount] : spendingMap ) {
        if ( balanceMap.find(minerId) == balanceMap.end() ) {
            std::cout << "Miner " << minerId << " not registered in balanceMap\n";
        }
        if (balanceMap[minerId] < spendingAmount ) {
            std::cout << "Miner " << minerId << " spending " << spendingAmount << " but only has " << balanceMap[minerId] << '\n';
            return false;
        }
    }
    return true;
}

bool BlockTree::validateBlock(Block & block) {
    BlockTreeNode * node = new BlockTreeNode(block);
    node->parent = this->current;
    node->height = this->current->height + 1;
    bool returnValue = validateChain(node);
    delete node;
    return returnValue;
}

void BlockTree::updateBalance(BlockTreeNode * node) {
    for ( Transaction & txn : node->block.transactions ) {
        if ( balanceMap.find(txn.sender) == balanceMap.end() ) balanceMap[txn.sender] = 0;
        if ( balanceMap.find(txn.receiver) == balanceMap.end() ) balanceMap[txn.receiver] = 0;
        if ( txn.type != TransactionType::COINBASE ) {
            balanceMap[txn.sender] -= txn.amount;
            if ( balanceMap[txn.sender] < 0 ) std::cout << "Balance of Miner " << txn.sender << " went negative due to switch due to block " << node->block.id << '\n';
            if ( id == 29 && txn.sender == 49 ) std::cout << "Balance updated to " << balanceMap[txn.sender] << " on block " << node->block.id << '\n';
            if ( id == 29 && txn.receiver == 49 ) std::cout << "Balance updated to " << balanceMap[txn.sender] << " on block " << node->block.id << '\n';
            balanceMap[txn.receiver] += txn.amount;
        } else {
            balanceMap[txn.receiver] += txn.amount;
        }
    }
}

int BlockTree::addBlock(Block & block, time_t arrivalTime) {
    if (blockToNode.find(block.id) != blockToNode.end()) {
        return -1;
    }
    BlockTreeNode * parent = blockToNode[block.parentID];
    if ( parent == nullptr ) {
        return -1;
    }
    std::cout << "Miner " << id << " trying to add block " << block.id << '\n';
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
        std::cout << "Block " << node->block.id << " Rejected by Miner " << this->id << '\n';
        delete node;
        return -1;
    }
}

void BlockTree::processTransaction(std::vector<Transaction> & transactions) {
    for ( Transaction & txn : transactions ) {
        if ( txn.type != TransactionType::COINBASE ) {
            balanceMap[txn.sender] -= txn.amount;
            if ( balanceMap[txn.sender] < 0 ) std::cout << "Balance of Miner " << txn.sender << " went negative in ProcessTransaction\n"; 
        }
        balanceMap[txn.receiver] += txn.amount;
        if ( id == 29 && txn.sender == 49 ) std::cout << "ProcessTransaction: Balance updated to " << balanceMap[txn.sender] << '\n';
        if ( id == 29 && txn.receiver == 49 ) std::cout << "ProcessTransaction: Balance updated to " << balanceMap[txn.receiver] << '\n';
    }
}

void BlockTree::deProcessTransactions(std::vector<Transaction> & transactions) {
    for ( Transaction & txn : transactions ) {
        if ( txn.type != TransactionType::COINBASE ) {
            balanceMap[txn.sender] += txn.amount;
        }
        balanceMap[txn.receiver] -= txn.amount;
        if ( balanceMap[txn.sender] < 0 ) std::cout << "Balance of Miner " << txn.sender << " went negative in deProcessTransactions\n"; 
        if ( id == 29 && txn.sender == 49 ) std::cout << "DeProcessTransaction: Balance updated to " << balanceMap[txn.sender] << '\n';
        if ( id == 29 && txn.receiver == 49 ) std::cout << "DeProcessTransaction: Balance updated to " << balanceMap[txn.receiver] << '\n';
    }
}

int BlockTree::getBalance(minerID_t id) {
    if (id == -1) {
        return balanceMap[id];
    }
    return balanceMap[id];
}

Block BlockTree::getCurrent() {
    return this->current->block;
}

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
        std::cout << "Latest Node: " << node->block.id << '\n';
        BlockTreeNode * node1 = node, * node2 = this->current;
        std::set<Transaction> memPoolInsert, memPoolErase;
        // memPoolErase.insert(node->block.transactions.begin(), node->block.transactions.end());
        // while ( node1->height != node2->height ) {
        while ( node1->height > node2->height ) {
            // if ( node1->height > node2->height ) {
                memPoolErase.insert(node1->block.transactions.begin(), node1->block.transactions.end());
                node1 = node1->parent;
                std::cout << "ProcessingTransactions in Block " << node1->block.id << '\n';
                processTransaction(node1->block.transactions);
            // }
            //  else {
            //     memPoolInsert.insert(node2->block.transactions.begin(), node2->block.transactions.end());
            //     node2 = node2->parent;
            //     deProcessTransactions(node2->block.transactions);
            // }
        }
        while ( node1 != node2 ) {
            memPoolErase.insert(node1->block.transactions.begin(), node1->block.transactions.end());
            std::cout << "ProcessingTransactions in Block " << node1->block.id << '\n';
            processTransaction(node1->block.transactions);
            node1 = node1->parent;
            
            memPoolInsert.insert(node2->block.transactions.begin(), node2->block.transactions.end());
            node2 = node2->parent;
            std::cout << "DeProcessingTransactions in Block " << node2->block.id << '\n';
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

void BlockTree::printTree(std::string filename) const {
    std::ofstream file(filename);
    if ( ! genesis || ! file.is_open() ) {
        return;
    }
    printSubTree(genesis, file);
    file.close();
}

void BlockTree::printSubTree(BlockTreeNode* node, std::ofstream & file) const {
    file << "( " << node->block.id << " " << node->arrivalTime << '\n';
    for (BlockTreeNode* child : node->children) {
        printSubTree(child, file);
    }
    file << ")" << '\n';
}

void BlockTree::printChain(BlockTreeNode* node) const {
    while (node) {
        std::cout << node->block.id << '\n';
        node = node->parent;
    }
}

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

    std::cout << "DOT file saved: " << filename << '\n';
}

void BlockTree::printBlock(BlockTreeNode* node, time_t arrivalTime) {
    file << "Block ID: " << node->block.id << ", Arrival Time: " << arrivalTime << ", Parent ID: " << node->parent->block.id << '\n';
}