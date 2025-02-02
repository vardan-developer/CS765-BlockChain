#include "transaction.hpp"

int Transaction::amount() const {
    return std::accumulate(in_utxos.begin(), in_utxos.end(), 0, [](int sum, Utxo & utxo) { return sum + utxo.amount; });
}

bool Transaction::isBalanceConsistent() const {
    return ( type == TransactionType::COINBASE ) ||
        (   std::accumulate(in_utxos.begin(), in_utxos.end(), 0, [](int sum, Utxo & utxo) { return sum + utxo.amount; }) ==
            std::accumulate(out_utxos.begin(), out_utxos.end(), 0, [](int sum, Utxo & utxo) { return sum + utxo.amount; }));
}