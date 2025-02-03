#ifndef BLOCKTREE_H
#define BLOCKTREE_H

#include "block.hpp"
#include "def.hpp"
#include "transaction.hpp"
#include <map>
#include <vector>
#include <set>
#include <time.h>

struct BlockTreeNode {
    Block block;
    time_t arrivalTime;
    int height;
    BlockTreeNode * parent;
    std::vector<BlockTreeNode*> children;

    BlockTreeNode(const Block & block);
    BlockTreeNode(const BlockTreeNode & other);
    BlockTreeNode & operator=(const BlockTreeNode & other);
};

class BlockTree {
    private:
        BlockTreeNode * genesis;
        BlockTreeNode * current;
        minerID_t id;
        std::map<minerID_t, int> balanceMap;
        std::map<blockID_t, BlockTreeNode *> blockToNode;

        bool validateChain(BlockTreeNode * node);

        BlockTreeNode * getLca(BlockTreeNode * node1, BlockTreeNode * node2);
        bool updateMempool(BlockTreeNode * node, std::set<Transaction> & memPool);
        void updateBalance(BlockTreeNode * node);
        void processTransaction(std::vector<Transaction> & transactions);
        void deProcessTransactions(std::vector<Transaction> & transactions);

        void printSubTree(BlockTreeNode* node, std::ofstream & file) const;
        void printBlock(BlockTreeNode* node) ;
        std::ofstream file;   

    public:
        BlockTree();
        BlockTree(minerID_t id);
        BlockTree(BlockTreeNode * genesis, minerID_t id);
        ~BlockTree();
        BlockTree(const BlockTree & other);
        BlockTree & operator=(const BlockTree & other);
        BlockTree(const BlockTree && other);
        BlockTree & operator=(const BlockTree && other);

        bool validateBlock(Block & block);
        int addBlock(Block & block);        // Returns the height of the longest chain after adding the block
        int switchToLongestChain(Block & block, std::set<Transaction> & memPool);
        int getBalance();
        Block getCurrent();
        void exportToDot(const std::string & filename) const;
        void printTree(std::string filename) const;
        void printChain(BlockTreeNode* node /* The bottom of the chain */) const; /* Prints the chain from the bottom to the genesis */
};



#endif