/** @defgroup group33 client_fabric
 *  @ingroup group3
 *  @{
 */
#ifndef CLIENT_FABRIC_H
#define CLIENT_FABRIC_H

#include <cstring>
#include <list>
#include <map>
#include <string>
#include <vector>

#include "adapter_interface/adapter_interface.h"
#include "json.hpp"

/**
 * @brief Client to communicate with TrustDBle smart contract on Hyperledger
 * Fabric Blockchain via Go-SDK.
 *
 * Provides functions to:
 *
 *      - put key value pair to ledger
 *      - get value of key from ledger
 *      - remove a key and its value
 *      - get all key value pairs on the ledger
 */
class FabricClient {
 public:
  //! Constructor
  FabricClient();
  //! Destructor
  ~FabricClient();

  // Mapped contract methods

  /**
   * @brief Initialize the client using the passe parameters
   *
   * @param channel_name Name of the fabric channel to connect to
   * @param contract_name Name of the contract to connect to for reads/writes
   * @param msp_id Id of the membership service provider
   * @param cert_path Path to the certificate of the client identity
   * @param key_path Path to a directoy with private keys of the client
   * @param tls_cert_path Path to the gateway peers tls certificate
   * @param gateway_peer Name of the gateway peer
   * @param peer_endpoint Adress of the gateway peer
   *
   * @return Returns a status code. 0 for success and 1 for errors
   */
  auto init(std::string channel_name, std::string contract_name,
            std::string msp_id, std::string cert_path, std::string key_path,
            std::string tls_cert_path, std::string peer_endpoint,
            std::string gateway_peer, std::string table_name) -> int;

  /**
   * @brief Put a batch of key-value pairs into the ledger
   *
   * @param batch Batch including multiple key-value pairs
   *
   * @return Returns a status code. 0 for success and 1 for errors
   */
  auto put(std::map<const BYTES, const BYTES> &batch) -> int;

  /**
   * @brief Reads a value specified by key from the ledger
   *
   * @param key Key of value to be read from the ledger
   * @param value Return parameter containing the value to the key
   *
   * @return Returns a status code. 0 for success and 1 for errors
   */
  auto get(const BYTES &key, BYTES &value) -> int;

  /**
   * @brief Reads all keys and values from the ledger
   *
   * @param values Return parameter containing a map of all
   * key-value pairs on the ledger
   *
   * @return Returns a status code. 0 for success and 1 for errors
   */
  auto getAll(std::map<const BYTES, BYTES> &values) -> int;

  /**
   * @brief Removes all given keys and their values from the ledger
   *
   * @param batch List of keys to be removed from ledger
   *
   * @return Returns a status code. 0 for success and 1 for errors
   */
  auto remove(std::list<BYTES> &batch) -> int;

  /**
   * @brief Closes client
   *
   * @return Returns a status code. 0 for success and 1 for errors
   */
  auto close() -> int;

  // public methods

  /**
   * @brief Checks if client is initialized
   *
   * @return true if client is initialized, false if not
   */
  [[nodiscard]] auto isInit() const -> bool;

 private:
  std::string channel_name_;
  std::string contract_name_;
  std::string msp_id_;
  std::string cert_path_;
  std::string key_path_;
  std::string tls_cert_path_;
  std::string gateway_peer_;
  std::string peer_endpoint_;
  std::string table_name_;
  bool isInitializied_ = false;
};
#endif  // CLIENT_FABRIC_H
/** @} */