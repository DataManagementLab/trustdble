#ifndef MANAGER_FABRIC_H
#define MANAGER_FABRIC_H

#include <algorithm>
#include <boost/log/trivial.hpp>
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid_io.hpp>

#include "bc_manager_interface/bc_manager_interface.h"
#include "config_fabric.h"
#include "networkconfig_fabric.h"

const int kMaxChannelNameLength = 250;
const std::string kChannelContainerLabel = "com.trustdble.channel";

class FabricManager : public BcManager {
 public:
  explicit FabricManager();
  ~FabricManager() override;

  /* BcManager methods to be implemented */

  auto init(const std::string &config_path) -> bool override;
  auto shutdown() -> bool override;

  auto create_network(const std::string &name,
                      std::unique_ptr<BcManagerNetworkConfig> &config)
      -> int override;

  auto join_network(const std::string &name,
                    std::unique_ptr<BcManagerNetworkConfig> &config)
      -> int override;

  auto leave_network(const std::string &name,
                     std::unique_ptr<BcManagerNetworkConfig> &config)
      -> int override;

 private:
  FabricConfig config_;
};
#endif  // MANAGER_FABRIC_H