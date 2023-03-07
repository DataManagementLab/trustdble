/** @defgroup group22 blocknumber
 *  @ingroup group2
 *  @{
 */
#pragma once

#include <exception>
#include <fstream>
#include <string>

/**
 * @brief This class is for the adapter_stub to be able to simulate blocknumbers in
 * Blockchains.
 */
class BlockNumber {
 public:
  /**
   * @brief Sets the path for the file, where the block number is persisted and
   * writes the initial block number 1 in that file.
   *
   * @param path the path for the file
   * @param blocksize how many transactions make up a block
   */
  void init(const std::string &path, int blocksize);

  /**
   * @brief Called on put requests to the blockchain stub to increment internal
   * transaction counter. The transaction counter is used to derive the current
   * blocknumber.
   */
  void incrementTransactionCount();

  /**
   * @brief The block number is computed via the transaction count (alias the
   * number of put requests seen) and the block size, which is the number of
   * transactions that form a block.
   *
   * @returns the current blocknumber
   */
  auto get() -> int;

 private:
  auto getTransactionCount() -> int;

  std::string blockNumberFilename_;
  std::ofstream blockNumberFile_;
  int blockSize_;
};
/** @} */