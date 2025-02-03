#include "blockTree.hpp"
#include "def.hpp"
#include "transaction.hpp"
#include <map>
#include <queue>

BlockTreeNode::BlockTreeNode(const Block & block) {
    this->block = block;
    this->parent = nullptr;
    this->children = std::vector<BlockTreeNode *>();
    this->height = 0;
}

BlockTreeNode::BlockTreeNode(const BlockTreeNode & other) {
    this->block = other.block;
    this->parent = other.parent;
    this->children = other.children;
    this->height = other.height;
}

BlockTreeNode & BlockTreeNode::operator=(const BlockTreeNode & other) {
    this->block = other.block;
    this->parent = other.parent;
    this->children = other.children;
    this->height = other.height;
    return *this;
}

BlockTree::BlockTree() {
    this->id = 0;
    this->genesis = nullptr;
    this->current = nullptr;
}

BlockTree::BlockTree(minerID_t id) {
    this->id = id;
    this->genesis = nullptr;
    this->current = nullptr;
}

BlockTree::BlockTree(BlockTreeNode * genesis, minerID_t id) {
    this->id = id;
    this->genesis = genesis;
    this->current = genesis;
}

BlockTree::~BlockTree() {
    std::queue<BlockTreeNode *> q;
    q.push(genesis);

    while ( !q.empty() ) {
        BlockTreeNode * node = q.front();
        for ( BlockTreeNode * child : node->children ) {
            q.push(child);
        }
        q.pop();
        delete node;
    }
}

BlockTree::BlockTree(const BlockTree & other) {
    this->id = other.id;
    this->genesis = new BlockTreeNode(*other.genesis);
    this->current = new BlockTreeNode(*other.current);
}

BlockTree & BlockTree::operator = (const BlockTree & other) {
    this->id = other.id;
    this->genesis = new BlockTreeNode(*other.genesis);
    this->current = new BlockTreeNode(*other.current);
    return *this;
}

BlockTree::BlockTree(const BlockTree && other) {
    this->id = other.id;
    this->genesis = other.genesis;
    this->current = other.current;
}

BlockTree & BlockTree::operator = (const BlockTree && other) {
    this->id = other.id;
    this->genesis = other.genesis;
    this->current = other.current;
    return *this;
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
        if ( balanceMap[minerId] < spendingAmount ) {
            return false;
        }
    }
    return true;
}

bool BlockTree::validateBlock(Block & block) {
    BlockTreeNode * node = new BlockTreeNode(block);
    node->parent = this->current;
    node->height = this->current->height + 1;
    return validateChain(node);
}

void BlockTree::updateBalance(BlockTreeNode * node) {
    for ( Transaction & txn : node->block.transactions ) {
        if ( txn.type != TransactionType::COINBASE ) {
            balanceMap[txn.sender] -= txn.amount;
            balanceMap[txn.receiver] += txn.amount;
        } else {
            balanceMap[txn.receiver] += txn.amount;
        }
    }
}

int BlockTree::addBlock(Block & block) {
    BlockTreeNode * node = new BlockTreeNode(block);
    BlockTreeNode * parent = blockToNode[block.parentID];
    node->parent = parent;
    node->height = parent->height + 1;
    if ( validateBlock(node->block) ) {
        blockToNode[block.id] = node;
        parent->children.push_back(node);
        updateBalance(node);
        return std::max(node->height, this->current->height);
    } else {
        delete node;
        return -1;
    }
}

void BlockTree::processTransaction(std::vector<Transaction> & transactions) {
    for ( Transaction & txn : transactions ) {
        if ( txn.type != TransactionType::COINBASE ) {
            balanceMap[txn.sender] -= txn.amount;
        }
        balanceMap[txn.receiver] += txn.amount;
    }
}

void BlockTree::deProcessTransactions(std::vector<Transaction> & transactions) {
    for ( Transaction & txn : transactions ) {
        if ( txn.type != TransactionType::COINBASE ) {
            balanceMap[txn.sender] += txn.amount;
        }
        balanceMap[txn.receiver] -= txn.amount;
    }
}

int BlockTree::getBalance() {
    return balanceMap[id];
}

Block BlockTree::getCurrent() {
    return this->current->block;
}

int BlockTree::switchToLongestChain(Block & block, std::set<Transaction> & memPool) {
    BlockTreeNode * node = blockToNode[block.id];
    if ( node == nullptr ) {
        return -1;
    }
    if ( node->height > this->current->height ) {
        BlockTreeNode * node1 = node, * node2 = this->current;
        std::set<Transaction> memPoolInsert, memPoolErase;
        while ( node1->height != node2->height ) {
            if ( node1->height > node2->height ) {
                memPoolInsert.insert(node1->block.transactions.begin(), node1->block.transactions.end());
                node1 = node1->parent;
                deProcessTransactions(node1->block.transactions);
            } else {
                memPoolErase.insert(node2->block.transactions.begin(), node2->block.transactions.end());
                node2 = node2->parent;
                processTransaction(node2->block.transactions);
            }
        }
        while ( node1 != node2 ) {
            memPoolInsert.insert(node1->block.transactions.begin(), node1->block.transactions.end());
            deProcessTransactions(node1->block.transactions);
            node1 = node1->parent;
            
            memPoolErase.insert(node2->block.transactions.begin(), node2->block.transactions.end());
            node2 = node2->parent;
            processTransaction(node2->block.transactions);
        }

        for ( Transaction txn : memPoolInsert ) {
            memPool.insert(txn);
        }
        for ( Transaction txn : memPoolErase ) {
            memPool.erase(txn);
        }

        this->current = node;
    }
    return this->current->height;
}