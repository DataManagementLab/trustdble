/** @defgroup group40 adapter_factory
 *  @ingroup group4
 *  @{
 */
#ifndef ADAPTER_FACTORY_H
#define ADAPTER_FACTORY_H

#include <map>
#include <memory>

#include "adapter_ethereum/adapter_ethereum.h"
#include "adapter_fabric/adapter_fabric.h"
#include "adapter_stub/adapter_stub.h"

/**
 * @brief Type of blockchain adapter
 */
enum BC_TYPE { kUnknownType, kStub, kEthereum, kFabric };

/**
 * @brief Type of blockchain adapter as string
 */
static const std::map<std::string, BC_TYPE> kBcTypeStrings = {
    {"STUB", kStub}, {"ETHEREUM", kEthereum}, {"FABRIC", kFabric}};

/**
 * @brief Interface definition to be used by storage engine to communicate with
 * concrete blockchain technology adapter, like Ethereum, Fabric, ...
 *
 */

class AdapterFactory {
 public:
  /**
   * @brief Factory Method to create an adapter of a specific blockchain type
   *
   * @param type Type of blockchain to communicate with e.g.: FABRIC, ETHEREUM
   * or STUB
   *
   * @return Pointer to created BC_Adapter object from sepcified type or nullptr
   */
  static auto create_adapter(BC_TYPE type) -> std::unique_ptr<BcAdapter>;

  /**
   * @brief Convert Adaptertype from string to enum BC_TYPE
   *
   * @param type adapter type as a string
   * @return BC_TYPE corresponding Enum-Type
   */
  static auto getBC_TYPE(const std::string &type) -> BC_TYPE;
};

#endif  // ADAPTER_FACTORY_H
/** @} */