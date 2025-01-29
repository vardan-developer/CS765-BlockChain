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
    std::vector<utxo> in_utxos, out_utxos;
    TransactionType type;
    Transaction(txnId_t id, std::vector<utxo> in_utxos, std::vector<utxo> out_utxos, TransactionType type):
    id(id), in_utxos(in_utxos), out_utxos(out_utxos), type(type)
    {}
};

#endif