#ifndef MANAGER_ETHEREUM_H
#define MANAGER_ETHEREUM_H

#include <curl/curl.h>

#include <boost/log/trivial.hpp>
#include <cstdint>
#include <cstdio>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <regex>
#include <string>
#include <thread>
#include <vector>

#include "bc_manager_interface/bc_manager_interface.h"
#include "config_ethereum.h"
#include "networkconfig_ethereum.h"

class EthereumManager : public BcManager {
 public:
  explicit EthereumManager();
  ~EthereumManager() override;

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
  EthereumConfig config_;
};
#endif  // MANAGER_ETHEREUM_H