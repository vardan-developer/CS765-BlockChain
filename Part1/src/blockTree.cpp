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
    currentHeight = 0;
}

BlockTree::BlockTree(const Block & genesisBlock, time_t arrivalTime) {
    genesis = new BlockTreeNode(genesisBlock, arrivalTime);
    current = genesis;
    currentHeight = 0;
}

BlockTree::BlockTree(const BlockTree & other) {
    genesis = new BlockTreeNode(*other.genesis);
    current = genesis;
    currentHeight = other.currentHeight;
}

BlockTree & BlockTree::operator=(const BlockTree & other) {
    if (this == &other) {
        return *this;
    }
    genesis = new BlockTreeNode(*other.genesis);
    current = genesis;
    currentHeight = other.currentHeight;
    return *this;
}

BlockTree::BlockTree(BlockTree && other) {
    genesis = other.genesis;
    current = other.current;
    currentHeight = other.currentHeight;
    other.genesis = nullptr;
    other.current = nullptr;
    other.currentHeight = 0;
}

BlockTree & BlockTree::operator=(BlockTree && other) {
    if (this == &other) {
        return *this;
    }
    genesis = other.genesis;
    current = other.current;
    currentHeight = other.currentHeight;
    other.genesis = nullptr;
    other.current = nullptr;
    other.currentHeight = 0;
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
    return current->block;
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

bool BlockTree::validateChain(BlockTreeNode* node) const {

}

int BlockTree::addBlock(const Block block, time_t arrivalTime) {
    
}
