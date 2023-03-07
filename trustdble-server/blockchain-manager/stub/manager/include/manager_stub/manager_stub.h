#ifndef MANAGER_STUB_H
#define MANAGER_STUB_H

#include "bc_manager_interface/bc_manager_interface.h"

#include "config_stub.h"
#include "networkconfig_stub.h"

class StubManager : public BcManager {
 public:
  explicit StubManager();
  ~StubManager() override;

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
    StubManagerConfig config_;
 
};
#endif  // MANAGER_STUB_H
