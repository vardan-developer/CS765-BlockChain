#ifndef UTXO_H
#define UTXO_H

#include "def.hpp"

struct utxo
{
    /* data */
    blockId_t block;
    txnId_t txn;
    uint8_t index;
    minerId_t owner;
    uint64_t amount;
    utxo(blockId_t block, txnId_t txn, uint8_t index, minerId_t owner, uint64_t amount): 
        block(block), 
        txn(txn), 
        index(index), 
        owner(owner), 
        amount(amount) {}
};


#endif