/*! \addtogroup group40
 *  @{
 */
#include "adapter_factory/adapter_factory.h"

auto AdapterFactory::create_adapter(BC_TYPE type)
    -> std::unique_ptr<BcAdapter> {
  std::unique_ptr<BcAdapter> adapter = nullptr;

  // Create new adapter of BC_TYPE type
  switch (type) {
    case kStub:
      adapter = std::make_unique<StubAdapter>();
      break;
    case kEthereum:
      adapter = std::make_unique<EthereumAdapter>();
      break;
    case kFabric:
      adapter = std::make_unique<FabricAdapter>();
      break;
    default:
      break;
  }

  return adapter;
}

auto AdapterFactory::getBC_TYPE(const std::string &type) -> BC_TYPE {
  // type.toUpperCase()
  auto it = kBcTypeStrings.find(type);
  if (it == kBcTypeStrings.end()) {
    return kUnknownType;
  }
  return it->second;
}
/** @} */