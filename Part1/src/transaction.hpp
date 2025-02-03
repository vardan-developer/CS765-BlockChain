#ifndef TRANSACTION_HPP
#define TRANSACTION_HPP

<<<<<<< HEAD
#include "def.hpp"
=======
#include "defs.hpp"
>>>>>>> ccc5a31cbd64c5237fa8fae2909df1e555927067

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
    size_t dataSize() const;
};

#endif