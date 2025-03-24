#ifndef TRANSACTION_HPP
#define TRANSACTION_HPP

#include "def.hpp"

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

    Transaction() : id(0), type(TransactionType::NORMAL), sender(0), receiver(0), amount(0) {}
    Transaction(txnID_t id, TransactionType type, minerID_t sender, minerID_t receiver, uint32_t amount);
    bool operator < (const Transaction& other) const {
        return id < other.id;
    }
    bool operator == (const Transaction& other) const {
        return id == other.id;
    }
    bool operator != (const Transaction& other) const {
        return id != other.id;
    }
    bool operator > (const Transaction& other) const {
        return id > other.id;
    }
    hash_t hash() const;
};

#endif