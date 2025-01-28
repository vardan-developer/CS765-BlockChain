#ifndef BLOCK_H
#define BLOCK_H

#include "transaction.h"

struct Block {
    int id;
    int height;
    int parent_id;
    std::vector<Transaction> transactions;
    time_t timestamp;

    Block(int id, int height, int parent_id, std::vector<Transaction> transactions, time_t timestamp):
        id(id),
        height(height),
        parent_id(parent_id),
        transactions(transactions),
        timestamp(timestamp)
    {}

    Block(int id, int height, int parent_id, time_t timestamp):
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