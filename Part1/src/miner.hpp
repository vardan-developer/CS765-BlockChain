#ifndef MINER_H
#define MINER_H

#include "event.hpp"
#include "utils.hpp"
#include "blockTree.hpp"

class Miner {
    private:
        minerId_t id;
        double hashPower;
        std::set<Transaction> memPool;
        BlockTree blockTree;
        Block currentBlock;    // Block on which the miner is currently working  
        int currentHeight;
        int amount;
        Block * currentScheduledBlock; //Block which is scheduled on main thread
        std::vector<Utxo> unspentUtxos;
        std::vector<minerId_t> neighbours;
        std::map<blockId_t, std::set<minerId_t>> blockToMiners;

        /*
        std::vector<Event> receiveEvent(Event):- Can receive events of type:
            1) RECEIVE_BROADCAST_TRANSACTION        - Function for receving transaction
            2) RECEIVE_BROADCAST_BLOCK              - Function for receiving block
            3) BROADCAST_BLOCK                      - A way of letting you know that your previous block has been broadcasted
            4) BROADCAST_TRANSACTION                - A way of letting you know that your previous transaction has been broadcasted
            5) BLOCK_CREATION                       - Confirmation 
        */
        std::vector<Event> receiveBroadcastTransaction(Event &event);
        std::vector<Event> receiveBroadcastBlock(Event &event);
        std::vector<Event> generateBlock(time_t prev_time);
        std::vector<Event> generateTransaction(time_t prev_time);
        std::vector<Event> confirmBlock(Event &event);
    public:
        Miner(int id, double hashPower, std::vector<minerId_t> neighbours);
        std::vector<Event> receiveEvent(Event &event);
};

#endif