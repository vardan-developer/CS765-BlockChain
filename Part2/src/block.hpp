#ifndef BLOCK_H
#define BLOCK_H

#include "def.hpp"
#include "transaction.hpp"

struct Block{
    blockID_t id;
    uint64_t height;
    blockID_t parentID;
    std::vector<Transaction> transactions;
    time_t timestamp;
    minerID_t owner;
    Block();
    Block(blockID_t id, uint64_t height, blockID_t parentID, std::vector<Transaction> transactions, time_t timestamp, minerID_t owner);
    Block(blockID_t id, uint64_t height, blockID_t parentID, time_t timestamp, minerID_t owner);
    Block(const Block& block);
    Block& operator=(const Block& block);
    bool operator==(const Block& block) const;
    bool operator!=(const Block& block) const;
    bool operator<(const Block& block) const;
    size_t dataSize() const;
};

#endif