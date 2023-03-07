#include "bc_manager_factory/bc_manager_factory.h"

using namespace blockchain_manager;

auto BcManagerFactory::create_manager(BC_TYPE type)
    -> std::unique_ptr<BcManager> {
  std::unique_ptr<BcManager> manager = nullptr;

  // Create new manager of BC_TYPE type
  switch (type) {
    case kEthereum:
      manager = std::make_unique<EthereumManager>();
      break;
    case kFabric:
      manager = std::make_unique<FabricManager>();
      break;
    case kStub:
      manager = std::make_unique<StubManager>();
      break;
    default:
      break;
  }

  return manager;
}

auto BcManagerFactory::getBC_TYPE(const std::string &type) -> BC_TYPE {
  // type.toUpperCase()
  auto it = kBcTypeStrings.find(type);
  if (it == kBcTypeStrings.end()) {
    return kUnknownType;
  }
  return it->second;
}