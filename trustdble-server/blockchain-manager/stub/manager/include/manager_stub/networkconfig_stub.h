#ifndef NETWORKCONFIG_STUB_H
#define NETWORKCONFIG_STUB_H

#include "bc_manager_interface/bc_manager_networkconfig.h"

class StubNetworkConfig : public BcManagerNetworkConfig {
 public:
  explicit StubNetworkConfig() = default;
  ~StubNetworkConfig() = default;

  /**
   * @brief Initializes the configuration with the provided parameters.
   *
   * @param network_name The name of the folder 
   * that is created under the stub-network-path to store all data of that stub network
   *
   * @return True if the configuration was successfully initialized otherwise false
   */
  auto init(const std::string& network_name) -> bool {
    config_.put("Network.network-name", network_name);
    return true;
   }

  /**
   * @brief The name of the stub network folder inside the stub-network.
   * The name of a network is always meta/data + server number + name of database.
   *
   * @return The data_path as string
   */
  auto network_name() -> std::string {
    return config_.get<std::string>("Network.network-name");
  }

};
#endif  // NETWORKCONFIG_STUB_H