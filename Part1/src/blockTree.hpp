#ifndef BLOCKTREE_HPP
#define BLOCKTREE_HPP

#include "block.hpp"
#include "def.hpp"

class BlockTreeNode {
    public:
        BlockTreeNode(Block block, time_t arrivalTime);
        BlockTreeNode(const BlockTreeNode & other);
        BlockTreeNode & operator=(const BlockTreeNode & other);
        BlockTreeNode* parent;
        std::vector<BlockTreeNode*> children;
        Block block;
        time_t arrivalTime;
        int height;
};

class BlockTree {
    private:
        BlockTreeNode* genesis;
        BlockTreeNode* current; // Points to the bottom of the current longest chain

        /*
            Validates that all transactions in the chain are consistent
        */
        bool validateChain(BlockTreeNode* node /* The bottom of the chain */, std::vector<Utxo *> & utxosUsedByNewNode) const;

        void printSubTree(BlockTreeNode* node, std::ofstream & file) const;
    public:
        std::unordered_map<blockId_t, BlockTreeNode*> blockIdToNode;
        BlockTreeNode* findLCA(BlockTreeNode* node1, BlockTreeNode* node2) const;

        BlockTreeNode* findLCA(Block node1, Block node2) const;
        BlockTree();

        BlockTree(const Block & genesisBlock, time_t arrivalTime);
        BlockTree(const BlockTree & other);
        BlockTree & operator=(const BlockTree & other);
        BlockTree(BlockTree && other);
        BlockTree & operator=(BlockTree && other);
        ~BlockTree();

        Block getCurrent() const;
        int getCurrentHeight() const;

        /*
            1) Checks if the block can be added to the desired chain (Checks if transactions used are valid)
            2) Returns -1 if the block cannot be added to the chain
            3) Returns the height of the chain if the block can be added to the chain
            4) Updates the current chain to the new longest chain
        */
        int addBlock(const Block block, time_t arrivalTime);

        void printTree(std::string filename) const;
        void printChain(BlockTreeNode* node /* The bottom of the chain */) const; /* Prints the chain from the bottom to the genesis */
        void exportToDot(const std::string & filename) const;
        bool verifyUtxo(Utxo & utxo) const;


};

#endif