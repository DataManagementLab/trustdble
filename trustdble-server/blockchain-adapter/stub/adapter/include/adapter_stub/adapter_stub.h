/** @defgroup group20 adapter_stub
 *  @ingroup group2
 *  @{
 */
#ifndef StubAdapter_STUB_ADAPTER_H
#define StubAdapter_STUB_ADAPTER_H

#include <boost/log/trivial.hpp>
#include <chrono>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <future>
#include <iomanip>
#include <iostream>
#include <regex>
#include <string>
#include <thread>
#include <vector>

#include "adapter_interface/adapter_interface.h"
#include "adapter_utils/encoding_helpers.h"
#include "blocknumber.h"
#include "config_stub.h"

/**
 * @brief This BC_Adapter is a simple implementation that stores data in text
 * files and provides all functionality of a bc adapter without the need to
 * deploy a real blockchain network.
 *
 */
class StubAdapter : public BcAdapter {
 public:
  //! Constructor
  explicit StubAdapter();
  //! Destructor
  ~StubAdapter() override;

  /**********************************************
   *  BC_Adapter methods to be implemented
   ***********************************************/

  auto init(const std::string &config_path) -> bool override;
  auto init(const std::string &config_path, const std::string &network_config)
      -> bool override;
  auto shutdown() -> bool override;

  /**
   * @brief Put a batch of key-value pairs into a file;
   * Key-value pairs are stored in a file odd lines key, even lines value.
   *
   * @param batch Batch including multiple key-value pairs
   *
   * @return Status code (0 on success, 1 on failure)
   */
  auto put(std::map<const BYTES, const BYTES> &batch) -> int override;

  /**
   * @brief Wrapper around put() method, that does lock verification (signature
   * and block timeout).
   *
   * @param key key to write to
   * @param value value to set
   * @param signature signature over the tuple of following values
   * @param transactionId identifying the transaction
   * @param nodeId identifying the node, where the transaction was issued from
   * @param blockTimeout maximum block number the lock is regarded valid
   * @returns 0 on success, 1 on failure
   */
  auto put(const std::string &key, const std::string &value,
           const std::string &signature, unsigned int transactionId,
           unsigned int nodeId, unsigned int blockTimeout) -> int;

  auto get(const BYTES &key, BYTES &result) -> int override;

  /**
   * @brief Wrapper around get() method, that does lock verification (signature
   * and block timeout).
   *
   * @param key key to read from
   * @param result value at given key is stored here
   * @param signature signature over the tuple of following values
   * @param transactionId identifying the transaction
   * @param nodeId identifying the node, where the transaction was issued from
   * @param blockTimeout maximum block number the lock is regarded valid
   * @returns 0 on success, 1 on failure
   */
  auto get(const std::string &key, std::string &result,
           const std::string &signature, unsigned int transactionId,
           unsigned int nodeId, unsigned int blockTimeout) -> int;

  auto get_all(std::map<const BYTES, BYTES> &results) -> int override;

  auto remove(const BYTES &key) -> int override;

  /**
   * @brief Wrapper around remove() method, that does lock verification
   * (signature and block timeout).
   *
   * @param key key to delete
   * @param signature signature over the tuple of following values
   * @param transactionId identifying the transaction
   * @param nodeId identifying the node, where the transaction was issued from
   * @param blockTimeout maximum block number the lock is regarded valid
   * @returns 0 on success, 1 on failure
   */
  auto remove(const std::string &key, const std::string &signature,
              unsigned int transactionId, unsigned int nodeId,
              unsigned int blockTimeout) -> int;

  auto create_table(const std::string &name, std::string &tableAddress)
      -> int override;
  auto load_table(const std::string &name, const std::string &tableAddress)
      -> int override;
  auto drop_table() -> int override;

  /**
   * @brief In this stub the block number is simulated by periodically
   * incrementing a counter in the background. The block number is used by the
   * lock manager to determine an appropriate block timeout.
   *
   * @returns the current block number of the blockchain or 0, when the
   * blocknumber file couldn't be opened
   */
  auto getCurrentBlockNumber() -> unsigned int;

 private:
  /**
   * @brief Verifies, that the given signature is valid for the given lock,
   * using the public key.
   *
   * @param signature the signature over the tuple of the row ID, transaction
   * ID, nodeID and block timeout
   * @param key the row ID to access
   * @param transactionId identifying the transaction
   * @param nodeId identifying the node from which the transaction was issued
   * @param blockTimeout counter symbolizing at which block number the lock
   * timed out and shouldn't be processed anymore
   * @returns true, if the signature is valid
   */
  static auto hasValidSignature(const std::string &signature,
                                const std::string &key,
                                unsigned int transactionId, unsigned int nodeId,
                                unsigned int blockTimeout) -> bool;

  /**
   * @brief The block timeout was sent together with a read or write request
   * from a transaction (transaction ID) on a given key (row ID). It is used to
   * check, if the read or write request should be further processed by the
   * blockchain storage engine or declined. If the block timeout number is
   * higher than the current block number, it means, that the corresponding lock
   * for that request was hold too long and that the request timed out.
   * Therefore, it shouldn't be processed.
   *
   * @param blockTimeout the blocknumber until the corresponding request is seen
   * as valid
   * @returns true, if the given block timeout is bigger than the current block
   * number, therefore the lock should not be further processed
   */
  auto hasLockTimedOut(unsigned int blockTimeout) -> bool;

  std::string tableName_;
  StubConfig config_;
  BlockNumber blockNumber_;
};
#endif  // StubAdapter_STUB_ADAPTER_H
/** @} */