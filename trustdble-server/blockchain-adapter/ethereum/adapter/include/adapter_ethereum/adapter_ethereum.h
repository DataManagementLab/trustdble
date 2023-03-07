/** @defgroup group10 adapter_ethereum
 *  @ingroup group1
 *  @{
 */
#ifndef ADAPTER_ETHEREUM_H
#define ADAPTER_ETHEREUM_H

#include <curl/curl.h>

#include <boost/log/trivial.hpp>
#include <cstdint>
#include <cstdio>
#include <fstream>
#include <iostream>
#include <mutex>		
#include <regex>
#include <string>
#include <thread>
#include <vector>

#include "adapter_interface/adapter_interface.h"
#include "config_ethereum.h"
#include "json.hpp"

// interval in ms to check if block is mined
#define MINING_CHECK_INTERVAL 200
// define waiting time in seconds in config file
#define WAITING_TIME_IN_SEC 1000
// keys and values of smart contrat are 32 byte and represented as hex
// strings with 64 chars. Each byte as 2 chars
#define VALUE_SIZE 64

#define ENCODED_BYTE_SIZE 16
// size for buffer to get return after executing node command
#define BUFFER_SIZE_EXEC 128


/**
 * @brief Defines parameters that are required to perform a RPC request to an
 * Ethereum node
 *
 */
struct RpcParams {
  //! Address (public key) from which the request is sent
  std::string from;
  //! Contract address the request is targeting
  std::string to;
  //! Content of the data
  std::string data;
  //! Contract method that the request will call
  std::string method;
  //! Gas limit of the request
  std::string gas;
  //! Gas price we are willing to pay for the request
  std::string gas_price;
  //! Quantity tag
  std::string quantity_tag;
  //! ID of the transaction that is created by this request
  std::string transaction_ID;
  //! The client nonce for the request
  unsigned long nonce{0};
  /**
   * @brief Default constructor for RpcParams
   *
   */
  RpcParams() = default;

  /**
   * @brief Overload the "==" operator
   *
   * @param params
   * @return true
   * @return false
   */
  auto operator==(const RpcParams &params) const -> bool {
    return transaction_ID == params.transaction_ID;
  }

  /**
   * @brief Overload the "!=" operator
   *
   * @param params
   * @return true
   * @return false
   */
  auto operator!=(const RpcParams &params) const -> bool {
    return transaction_ID != params.transaction_ID;
  }

  /**
   * @brief Overload the ">" operator
   *
   * @param params
   * @return true
   * @return false
   */
  auto operator>(const RpcParams &params) const -> bool {
    return transaction_ID > params.transaction_ID;
  }

  /**
   * @brief Overload the "<" operator
   *
   * @param params
   * @return true
   * @return false
   */
  auto operator<(const RpcParams &params) const -> bool {
    return transaction_ID < params.transaction_ID;
  }

  /**
   * @brief Overload the ">=" operator
   *
   * @param params
   * @return true
   * @return false
   */
  auto operator>=(const RpcParams &params) const -> bool {
    return transaction_ID >= params.transaction_ID;
  }

  /**
   * @brief Overload the "<=" operator
   *
   * @param params
   * @return true
   * @return false
   */
  auto operator<=(const RpcParams &params) const -> bool {
    return transaction_ID <= params.transaction_ID;
  }
};

/**
 * @brief BC_Adapter implementation for Ethereum.
 *
 */
class EthereumAdapter : public BcAdapter {
 public:
  static std::mutex m;
  //! Constructor
  explicit EthereumAdapter();
  //! Destructor
  ~EthereumAdapter() override;

  /* BC_Adapter methods to be implemented */

  auto init(const std::string &config_path) -> bool override;
  auto init(const std::string &config_path, const std::string &network_config)
      -> bool override;
  auto shutdown() -> bool override;
  /**
   * @brief Put a batch of key-value pairs into the Ethereum blockchain using
   * Rpc calls to the Ethereum endpoint; Transactions are sent to the Blockchain
   * and then we check whether a transaction was successfully stored on the
   * blockchain. Successful transactions are removed from the batch while failed
   * transactions remain in the batch.
   *
   * @param batch Batch including multiple key-value pairs; Succesfully inserted
   * key-value pairs are removed from the batch
   *
   * @return Status code (0 on success, 1 on failure batch contains remaining
   * key-value pairs)
   */
  auto put(std::map<const BYTES, const BYTES> &batch) -> int override;
    
  /**
   * @brief Put a batch of key-value pairs into the Ethereum blockchain using
   * Rpc calls to the Ethereum endpoint. 
   *
   * @param batch Batch including multiple key-value pairs
   *
   * @return Status code 0 on run without error
   */
  auto put_batch(std::map<const BYTES, const BYTES> &batch) -> int;
  auto get(const BYTES &key, BYTES &result) -> int override;
  auto get_all(std::map<const BYTES, BYTES> &results) -> int override;
  auto remove(const BYTES &key) -> int override;

  auto create_table(const std::string &name, std::string &tableAddress)
      -> int override;
  auto load_table(const std::string &name, const std::string &tableAddress)
      -> int override;
  auto drop_table() -> int override;

 private:
  std::string tableName_;
  std::string accountAddress_;
  std::string storedContractAddress_;
  EthereumConfig config_;

  CURL *curl_;
  size_t max_waiting_time_;
  std::atomic_uint64_t nonce_;

  /**
   * @brief Verify configuration path
   *
   * @return true if successfull otherwise false
   */
  static auto verify_config_path(const std::string &config_path) -> bool;

  /**
   * @brief Verify network configuration
   *
   * @return true if successfull otherwise false
   */
  static auto verify_network_config(const std::string &network_config) -> bool;

  /**
   * @brief Update nonce for reading operations
   *
   * @return true if successfull otherwise false
   */
  auto update_nonce() -> bool;

  /**
   * @brief Initialize adapter after config is set
   *
   * @return true if successfull otherwise false
   */
  auto init() -> bool;

  /**
   * @brief Helper-Method to do a RPC call to the blockchain
   *
   * @param params RpcParams struct containing parameters of the call
   *
   * @param set_gas True gas will be set otherwise not
   *
   * @return Response of the blockchain
   */
  auto call(RpcParams params, bool set_gas) -> std::string;

  /**
   * @brief Helper-Method to do a RPC call to the blockchain
   *
   * @param params Json-formatted string containing parameters of the call
   *
   * @param method RPC-Method that is call on the blockchain e.g.:
   * "eth_sendTransaction", "eth_call", ...
   *
   * @return Response of the blockchain
   */
  auto call(std::string &params, std::string &method) -> std::string;

  /**
   * @brief Helper-Method to periodically poll the blockchain to check if a
   * transaction was mined. The poll interval is defined by
   * MINING_CHECK_INTERVAL constant. It aborts if max-waiting-time is reached.
   *
   * @param transaction_ID The ID of the transaction that is being checked to
   * see if it was mined
   *
   * @return Transaction response of the blockchain. If the transaction was
   * mined it contains the block number and hash, otherwise block number and
   * hash are null;
   */
  auto check_mining_result(std::string &transaction_ID) -> std::string;

  /**
   * @brief Helper-Method to check the transation state after mining the
   * transaction
   *
   * @param transcation_ID The ID of the transaction whose state is to be
   * checked
   *
   * @return True if transaction was successful, otherwise false
   *    */
  auto check_transaction_receipt(std::string &transaction_ID) -> bool;

  /**
   * @brief Helper-Method to parse a RpcParam struct to json
   *
   * @param params RPC parameter as RpcParam struct
   *
   * @return A json formatted string containing the parameters
   */
  static auto parse_params_to_json(const RpcParams &params) -> std::string;

  /**
   * @brief Helper-Method to parse the transaction id from a transaction
   * response after sending a transaction to the blockchain.
   *
   * @param read_buffer_call Raw response from blockchain
   *
   * @param[out] json_response Response is parsed into this json object
   */
  static void parseTX_response(const std::string &read_buffer_call,
                               nlohmann::json &json_response);

  /**
   * @brief Helper-Method to convert a string to 32byte size. It is appened with
   * '0' (and the first byte will be the size of the data).
   *
   * @param data The string to be padded.
   *
   * @return String with size of 32byte in hex (64chars)
   */
  static auto convert_to_32byte(const std::string &data) -> std::string;

  /**
   * @brief Helper-Method to split and parse concatenated hex-encoded response
   * from blockchain contract when doing a table scan. It extracts a key list
   * and value list that are returned as a tuple vector.
   *
   * @param response Blockchain contract response as concatenated hex-encoded
   * string
   *
   * @param split_length Length of an element in the response.
   * Default=VALUE_SIZE of ethereum (64)
   *
   * @return A vector of string tuples containing key and value
   */
  static auto split(const std::string &response, int split_length = VALUE_SIZE)
      -> std::map<const BYTES, BYTES>;

  /**
   * @brief Callback function for curl
   */
  static auto write_callback(char *contents, size_t size, size_t nmemb,
                             void *userp) -> size_t;

  /**
   * @brief Helper-Method to do a RPC call to the blockchain for batch
   * processing
   *
   * @param batch Map that consists of a RpcParams struct and a boolean for
   * setting gas or not
   *
   * @param key_map Map that consists of a RpcParams struct and its
   * corresponding input key that is associated with these parameters
   *
   * @return Vector that contains the keys where processing failed
   */
  auto createRpcBatch(std::map<RpcParams, bool> batch,
                      std::map<RpcParams, std::string> key_map)
      -> std::pair<std::map<std::string, std::string>,
                   std::map<std::string, std::string>>;
  
  /**
   * @brief Helper-Method to do a RPC call to the blockchain for batch
   * processing
   *
   * @param batch Map that consists of a Json-formatted string containing
   * parameters of the call and the RPC-Method that is called on the Blockchain,
   * e.g., "eth_sendTransaction"
   *
   * @param key_map Map that consists of a Json-formatted string containing
   * parameters of the call and the original key that is associated with these
   * parameters
   *
   * @return Vector that contains the keys where processing failed
   */
  auto sendRpcBatch(std::map<std::string, std::string> batch,
                    std::map<std::string, std::string> key_map)
      -> std::vector<std::string>;
};
#endif  // ADAPTER_ETHEREUM_H
/** @} */