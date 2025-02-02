#ifndef BLOCK_H
#define BLOCK_H

#include "transaction.hpp"

struct Block {
    blockId_t id;
    uint64_t height;
    blockId_t parent_id;
    std::vector<Transaction> transactions;      // Remember to call transactions.shrink_to_fit() after adding all required transactions to reduce block size
    time_t timestamp;


    Block();

    Block(blockId_t id, uint64_t height, blockId_t parent_id, std::vector<Transaction> transactions, time_t timestamp):
        id(id),
        height(height),
        parent_id(parent_id),
        transactions(transactions),
        timestamp(timestamp)
    {}

    Block(blockId_t id, uint64_t height, blockId_t parent_id, time_t timestamp):
        id(id),
        height(height),
        parent_id(parent_id),
        timestamp(timestamp)
    {}

    Block(const Block & other);

    Block & operator=(const Block & other);

    bool operator < (const Block& other) const;

    size_t dataSize() const;
};

#endif