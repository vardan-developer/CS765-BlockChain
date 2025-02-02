#include "transaction.hpp"

Transaction::Transaction(txnId_t id, std::vector<Utxo> in_utxos, std::vector<Utxo> out_utxos, TransactionType type) {
    this->id = id;
    this->in_utxos = in_utxos;
    this->out_utxos = out_utxos;
    this->type = type;
}

int Transaction::amount() const {
    return std::accumulate(out_utxos.begin(), out_utxos.end(), 0, [](int sum, Utxo & utxo) { return sum + utxo.amount; });
}

bool Transaction::isBalanceConsistent() const {
    return ( type == TransactionType::COINBASE && std::accumulate(out_utxos.begin(), out_utxos.end(), 0, [](int sum, Utxo & utxo) { return sum + utxo.amount; }) == MINING_REWARD) ||
        (   std::accumulate(in_utxos.begin(), in_utxos.end(), 0, [](int sum, Utxo & utxo) { return sum + utxo.amount; }) ==
            std::accumulate(out_utxos.begin(), out_utxos.end(), 0, [](int sum, Utxo & utxo) { return sum + utxo.amount; }));
}

size_t Transaction::dataSize() const {
    return sizeof(Transaction) + std::accumulate(in_utxos.begin(), in_utxos.end(), (size_t) 0, [](int sum, Utxo & utxo) { return sum + utxo.dataSize(); }) + std::accumulate(out_utxos.begin(), out_utxos.end(), (size_t) 0, [](int sum, Utxo & utxo) { return sum + utxo.dataSize(); }); 
}