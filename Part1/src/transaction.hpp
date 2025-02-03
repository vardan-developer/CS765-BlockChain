#ifndef TRANSACTION_HPP
#define TRANSACTION_HPP

#include <"def.hpp">

enum class TransactionType{
    NORMAL,
    COINBASE
};

struct Transaction{
    txnID_t id;
    TransactionType type;
    minerID_t sender;
    minerID_t receiver;
    uint32_t amount;

    Transaction(txnID_t id, TransactionType type, minerID_t sender, minerID_t receiver, uint32_t amount);
};

#endif