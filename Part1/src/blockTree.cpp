#include "blockTree.hpp"

BlockTreeNode::BlockTreeNode(Block block, time_t arrivalTime) {
    this->block = block;
    this->arrivalTime = arrivalTime;
    height = 0;
    parent = nullptr;
    children = std::vector<BlockTreeNode*>();
}

BlockTreeNode::BlockTreeNode(const BlockTreeNode & other) {
    this->block = other.block;
    this->arrivalTime = other.arrivalTime;
    this->height = other.height;
    this->parent = other.parent;
    this->children = other.children;
}

BlockTreeNode & BlockTreeNode::operator=(const BlockTreeNode & other) {
    this->block = other.block;
    this->arrivalTime = other.arrivalTime;
    this->height = other.height;
    this->parent = other.parent;
    this->children = other.children;
    return *this;
}

BlockTree::BlockTree() {
    genesis = nullptr;
    current = nullptr;
}

BlockTree::BlockTree(const Block & genesisBlock, time_t arrivalTime) {
    genesis = new BlockTreeNode(genesisBlock, arrivalTime);
    current = genesis;
}

BlockTree::BlockTree(const BlockTree & other) {
    genesis = new BlockTreeNode(*other.genesis);
    current = genesis;
}

BlockTree & BlockTree::operator=(const BlockTree & other) {
    if (this == &other) {
        return *this;
    }
    genesis = new BlockTreeNode(*other.genesis);
    current = genesis;
    return *this;
}

BlockTree::BlockTree(BlockTree && other) {
    genesis = other.genesis;
    current = other.current;
    other.genesis = nullptr;
    other.current = nullptr;
}

BlockTree & BlockTree::operator=(BlockTree && other) {
    if (this == &other) {
        return *this;
    }
    genesis = other.genesis;
    current = other.current;
    other.genesis = nullptr;
    other.current = nullptr;
    return *this;
}

BlockTree::~BlockTree() {
    std::stack<BlockTreeNode*> stack;
    stack.push(genesis);
    while (!stack.empty()) {
        BlockTreeNode* node = stack.top();
        stack.pop();
        for (BlockTreeNode* child : node->children) {
            stack.push(child);
        }
        delete node;
    }
}

Block BlockTree::getCurrent() const {
    return this->current->block;
}

int BlockTree::getCurrentHeight() const {
    return this->current->height;
}

BlockTreeNode* BlockTree::findLCA(BlockTreeNode* node1, BlockTreeNode* node2) const {
    while (node1 != node2) {
        if (node1->height > node2->height) {
            node1 = node1->parent;
        } else {
            node2 = node2->parent;
        }
    }
    return node1;
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
    file << "( " << node->block.id << " " << node->arrivalTime << std::endl;
    for (BlockTreeNode* child : node->children) {
        printSubTree(child, file);
    }
    file << ")" << std::endl;
}

void BlockTree::printChain(BlockTreeNode* node) const {
    while (node) {
        std::cout << node->block.id << std::endl;
        node = node->parent;
    }
}

bool BlockTree::validateChain(BlockTreeNode* node, std::vector<Utxo *> & utxosUsedByNewNode) const {

    // Verifying each transaction
    for ( Transaction & transaction : node->block.transactions ) {

        // Sum of input utxo amount is not equal to sum of output utxo amount
        if ( ! transaction.isBalanceConsistent() ) {
            std::cout << "The block contains inconsistent transaction\n";
            return false;
        }

        // Verifying each input Utxos used in the transaction
        for ( Utxo & utxo : transaction.in_utxos ) {

            // Block containing the utxo is not in our blockchain
            if ( blockIdToNode.find(utxo.block) == blockIdToNode.end() ) {
                std::cout << "The block of the utxo which the new transaction claims to use is not present in our blockchain\n";
                return false;
            }

            // Getting the block object of the utxo
            Block & prevUtxoBlock = (blockIdToNode.at(utxo.block))->block;

            // Finding the transaction of the utxo
            Transaction * prevUtxoTransaction = & *std::find_if(prevUtxoBlock.transactions.begin(), prevUtxoBlock.transactions.end(), [&utxo](Transaction & txn) { return txn.id == utxo.txn; });       // TODO: Does this actually capture the transaction stored in the actual blockchain OR points to just a copy

            // Actually finding the Utxo the node claims to use
            Utxo * prevUtxo = & prevUtxoTransaction->out_utxos[utxo.index];

            // Verify if given utxo object is consistent with the stored utxo object
            if ( *prevUtxo == utxo ) {
                return false;
            }

            // Iterating over all the blocks which are using this utxo and verifying if these are not ancestor of the new node
            for ( blockId_t & blockConsumingUtxo : prevUtxo->consumedBy ) {

                // Cannot find the block in our blockchain
                // This also takes care if our block tries to use the same utxo multiple times
                if ( blockIdToNode.find(blockConsumingUtxo) == blockIdToNode.end() ) {
                    std::cout << "Something horribly wrong went down\n";
                    return false;
                }
                if ( blockIdToNode.at(blockConsumingUtxo) == this->findLCA(node, blockIdToNode.at(blockConsumingUtxo)) ) {
                    // Invalid transaction as utxo used by an ancestor                    
                    return false;
                }
            }

            // Registering the new block as one of the consumers of this Utxo
            prevUtxo->consumedBy.push_back(node->block.id);

            // Bookkeeping for roolback purposes
            utxosUsedByNewNode.push_back(prevUtxo);
        }
    }
    return true;
}

int BlockTree::addBlock(const Block block, time_t arrivalTime) {

    // Making a Tree Node object for our new block
    BlockTreeNode * blockTreeNode = new BlockTreeNode (block, arrivalTime);
    BlockTreeNode * parentBlock = blockIdToNode[block.parent_id];
    blockTreeNode->parent = parentBlock;
    blockTreeNode->height = parentBlock->height + 1;
    std::vector<Utxo *> utxosUsedByNewNode;

    if ( this->validateChain(blockTreeNode, utxosUsedByNewNode) ) {
        // Registering the new node in mappings
        blockIdToNode[blockTreeNode->block.id] = blockTreeNode;
        parentBlock->children.push_back(blockTreeNode);
        std::cout << "Block added succesfully in blockchain!\n";
    } else {
        for ( Utxo * prevAcceptedUtxo : utxosUsedByNewNode ) {
            prevAcceptedUtxo->consumedBy.pop_back();
        }
        delete blockTreeNode;
        std::cout << "Block rejected from blockchain!\n";
        return -1;
    }

    // Updating current head of blockchain
    if ( blockTreeNode->height > this->current->height ) {
        this->current = blockTreeNode;
    }

    return this->current->height;
}

bool BlockTree::verifyUtxo(Utxo & utxo) const {
    BlockTreeNode * utxoBlock = blockIdToNode.at(utxo.block);
    Transaction * utxoTransaction = & *std::find_if(utxoBlock->block.transactions.begin(), utxoBlock->block.transactions.end(), [&utxo](Transaction & txn) { return txn.id == utxo.txn; });
    Utxo * storedUtxo = & utxoTransaction->out_utxos[utxo.index];
    if ( * storedUtxo != utxo ) {
        return false;
    }
    if ( utxoBlock == this->findLCA(utxoBlock, this->current) || this->current == this->findLCA(utxoBlock, this->current) ) {
        return false;
    }
    for ( auto blockConsumingUtxo : storedUtxo->consumedBy ) {
        if ( utxoBlock == this->findLCA(utxoBlock, blockIdToNode.at(blockConsumingUtxo)) || blockIdToNode.at(blockConsumingUtxo) == this->findLCA(utxoBlock, blockIdToNode.at(blockConsumingUtxo)) ) {
            return false;
        }
    }
    return true;
}