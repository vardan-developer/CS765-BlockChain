#ifndef BLOCKTREE_H
#define BLOCKTREE_H

#include "block.hpp"
#include "def.hpp"
#include "transaction.hpp"
#include <ctime>
#include <map>
#include <vector>
#include <set>
#include <time.h>

// Node structure for the blockchain tree
struct BlockTreeNode {
    Block block;                // The block data
    time_t arrivalTime;        // Time when the block was received
    int height;                // Height of the block in the tree
    BlockTreeNode * parent;    // Pointer to parent block
    std::vector<BlockTreeNode*> children;  // List of child blocks
    
    // Constructors
    BlockTreeNode();
    BlockTreeNode(Block block, int height, time_t arrivalTime);
    BlockTreeNode(const Block & block);
    BlockTreeNode(const BlockTreeNode & other);
    BlockTreeNode & operator=(const BlockTreeNode & other);
};

class BlockTree {
    private:
        BlockTreeNode * genesis;    // Root of the blockchain tree
        BlockTreeNode * current;    // Current head of the longest valid chain
        minerID_t id;              // ID of the miner owning this tree
        std::map<minerID_t, int> balanceMap;           // Tracks balance for each miner
        std::map<blockID_t, BlockTreeNode *> blockToNode;  // Maps block IDs to their nodes
        
        // Tree manipulation helpers
        BlockTreeNode * deepCopy(BlockTreeNode * root, BlockTreeNode * other);
        bool validateChain(BlockTreeNode * node);
        
        // Chain management
        BlockTreeNode * getLca(BlockTreeNode * node1, BlockTreeNode * node2);  // Find Lowest Common Ancestor
        bool updateMempool(BlockTreeNode * node, std::set<Transaction> & memPool);
        void updateBalance(BlockTreeNode * node);
        void processTransaction(std::vector<Transaction> & transactions);
        void deProcessTransactions(std::vector<Transaction> & transactions);
        
        // Visualization and analysis helpers
        void printSubTree(BlockTreeNode* node, std::ofstream & file) const;
        void printBlock(BlockTreeNode* node, time_t arrivalTime);
        std::unordered_set<BlockTreeNode*> findMainChain();
        void findLeaves(BlockTreeNode* node, std::unordered_set<BlockTreeNode*>& mainChain, std::vector<BlockTreeNode*>& leaves);
        int findDistanceToMainChain(BlockTreeNode* leaf, std::unordered_set<BlockTreeNode*>& mainChain);
        std::ofstream file;   
        std::set<std::pair<Block, time_t>> cachedChildren;  // Store blocks for later processing

    public:
        // Constructors and assignment operators
        BlockTree();
        BlockTree(minerID_t id);
        BlockTree(minerID_t id, Block genesisBlock);
        ~BlockTree();
        BlockTree(const BlockTree & other);
        BlockTree & operator=(const BlockTree & other);
        BlockTree(BlockTree && other);
        BlockTree & operator=(BlockTree && other);

        // Core blockchain operations
        bool validateBlock(Block & block);  // Verify if a block is valid
        int addBlock(Block & block, time_t arrivalTime);  // Add block and return longest chain height
        bool switchToLongestChain(Block & block, std::set<Transaction> & memPool);
        int getBalance(minerID_t id = -1);  // Get balance for specific miner or self
        Block getCurrent();  // Get current head block
        Block getNextBlock(blockID_t blockID);
        // Visualization and analysis methods
        void exportToDot(const std::string & filename) const;  // Export tree in DOT format
        void printTree(std::string filename) const;
        void printChain(BlockTreeNode* node) const;  // Print chain from node to genesis
        void printSummary(bool fast, bool highCpu, long long totalBlocksGenerated);
        Block addCachedChild();  // Process a cached block
        long long getMainChainBlockCount();  // Count blocks in main chain
        float getRatio(long long totalBlocksGenerated);  // Calculate acceptance ratio
        float averageBranchLength();  // Calculate average branch length
        Block getBlock(blockID_t blockID);  // Get block by ID
};



#endif