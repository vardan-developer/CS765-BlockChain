#ifndef TRANSACTION_H
#define TRANSACTION_H

#include "def.hpp"
#include "utxo.hpp"

enum class TransactionType {
    NORMAL,
    COINBASE
};

struct Transaction {
    txnId_t id;
    std::vector<Utxo> in_utxos, out_utxos;
    TransactionType type;
    Transaction(txnId_t id, std::vector<Utxo> in_utxos, std::vector<Utxo> out_utxos, TransactionType type);
    int amount() const;
    bool isBalanceConsistent() const;
    size_t dataSize() const;
};

#endif