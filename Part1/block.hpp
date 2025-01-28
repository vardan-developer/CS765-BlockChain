#ifndef BLOCK_H
#define BLOCK_H

#include "transaction.h"

struct Block {
    uint64_t id;
    uint64_t height;
    uint64_t parent_id;
    std::vector<Transaction> transactions;
    time_t timestamp;

    Block(uint64_t id, uint64_t height, uint64_t parent_id, std::vector<Transaction> transactions, time_t timestamp):
        id(id),
        height(height),
        parent_id(parent_id),
        transactions(transactions),
        timestamp(timestamp)
    {}

    Block(uint64_t id, uint64_t height, uint64_t parent_id, time_t timestamp):
        id(id),
        height(height),
        parent_id(parent_id),
        timestamp(timestamp)
    {}

    bool operator < (const Block& other) const {
        return id < other.id;
    }
};

#endif