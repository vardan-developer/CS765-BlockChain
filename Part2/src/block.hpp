#ifndef BLOCK_H
#define BLOCK_H

#include "def.hpp"
#include "transaction.hpp"

/**
 * @brief Represents a block in the blockchain
 * 
 * The Block structure contains all necessary information for a block
 * in the blockchain, including its ID, height, parent block reference,
 * transactions, timestamp, and owner information.
 */
struct Block {
    blockID_t id;        // Unique identifier for the block
    uint64_t height;     // Block height in the blockchain (distance from genesis block)
    blockID_t parentID;  // ID of the parent block (forms blockchain linkage)
    std::vector<Transaction> transactions;  // List of transactions contained in this block
    time_t timestamp;    // Time when the block was created
    minerID_t owner;     // ID of the miner who created this block

    // Default constructor
    Block();

    /**
     * @brief Full constructor with all parameters
     * @param id Block identifier
     * @param height Block height in chain
     * @param parentID Parent block identifier
     * @param transactions Vector of transactions
     * @param timestamp Block creation time
     * @param owner Miner identifier
     */
    Block(blockID_t id, uint64_t height, blockID_t parentID, 
          std::vector<Transaction> transactions, time_t timestamp, minerID_t owner);

    /**
     * @brief Constructor without transactions
     * @param id Block identifier
     * @param height Block height in chain
     * @param parentID Parent block identifier
     * @param timestamp Block creation time
     * @param owner Miner identifier
     */
    Block(blockID_t id, uint64_t height, blockID_t parentID, 
          time_t timestamp, minerID_t owner);

    /**
     * @brief Copy constructor
     * @param block Block to copy from
     */
    Block(const Block& block);

    /**
     * @brief Assignment operator
     * @param block Block to assign from
     * @return Reference to this block
     */
    Block& operator=(const Block& block);

    /**
     * @brief Equality comparison operator
     * @param block Block to compare with
     * @return true if blocks are equal
     */
    bool operator==(const Block& block) const;

    /**
     * @brief Inequality comparison operator
     * @param block Block to compare with
     * @return true if blocks are not equal
     */
    bool operator!=(const Block& block) const;

    /**
     * @brief Less than comparison operator
     * @param block Block to compare with
     * @return true if this block is less than the parameter
     */
    bool operator<(const Block& block) const;

    /**
     * @brief Calculates the size of the block's data
     * @return Size of the block in bytes
     */
    size_t dataSize() const;

    hash_t hash() const;
};

#endif