#ifndef UTXO_H
#define UTXO_H

#include "def.hpp"

struct Utxo
{
    /* data */
    blockId_t block;
    txnId_t txn;
    uint8_t index;
    minerId_t owner;
    uint64_t amount;
    std::vector<blockId_t> consumedBy;
    Utxo(blockId_t block, txnId_t txn, uint8_t index, minerId_t owner, uint64_t amount): 
        block(block), 
        txn(txn), 
        index(index), 
        owner(owner), 
        amount(amount),
        consumedBy(std::vector<blockId_t> (0, 0))
    {}

    bool operator == (const Utxo & other) const {
        return (block == other.block) &&
               (txn == other.txn) &&
               (index == other.index) &&
               (owner == other.owner) &&
               (amount == other.amount);
    }

    bool operator != (const Utxo & other) const {
        return !(*this == other);
    }

    size_t dataSize() const {
        return sizeof(Utxo) + consumedBy.capacity() * sizeof(blockId_t);
    }
};


#endif