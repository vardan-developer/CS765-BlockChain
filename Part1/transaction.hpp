#ifndef TRANSACTION_H
#define TRANSACTION_H

#include "def.h"

enum class TransactionType {
    NORMAL,
    COINBASE
};

struct Transaction {
    txnId id;
    std::vector<txnId> utxos;
    minerId sender;
    minerId receiver;
    uint64_t amount;
    TransactionType type;
    Transaction(txnId id, std::vector<txnId> utxos, minerId sender, minerId receiver, uint64_t amount, TransactionType type):
        id(id),
        utxos(utxos),
        sender(sender),
        receiver(receiver),
        amount(amount),
        type(type)
    {}
};

#endif