#ifndef MANAGER_FACTORY_H
#define MANAGER_FACTORY_H

#include <map>
#include <memory>

#include "manager_ethereum/manager_ethereum.h"
#include "manager_fabric/manager_fabric.h"
#include "manager_stub/manager_stub.h"

namespace blockchain_manager {

/**
 * Type of blockchain
 */
enum BC_TYPE { kUnknownType, kEthereum, kFabric, kStub };

/**
 * Type of blockchain as string
 */
static const std::map<std::string, BC_TYPE> kBcTypeStrings = {
    {"ETHEREUM", kEthereum}, {"FABRIC", kFabric}, {"STUB", kStub}};

/**
 * @brief Factory class to create BlockchainManager instances.
 *
 */
class BcManagerFactory {
 public:
  /**
   * @brief Factory Method to create a manager of a specific blockchain type
   *
   * @param type Type of blockchain e.g.: ETHEREUM
   *
   * @return Pointer to created BcManager object of specified type or nullptr if
   * type is not supported
   */
  static auto create_manager(BC_TYPE type) -> std::unique_ptr<BcManager>;

  /**
   *  Converts blockchain type from string to enum BC_TYPE
   */
  static auto getBC_TYPE(const std::string &type) -> BC_TYPE;
};

}  // namespace blockchain_manager

#endif  // MANAGER_FACTORY_H
