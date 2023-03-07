#ifndef CONFIG_FABRIC_H
#define CONFIG_FABRIC_H

#include "bc_manager_interface/bc_manager_config.h"

class FabricConfig : public BcManagerConfig {
 public:
  explicit FabricConfig() = default;
  ~FabricConfig() override = default;

  /**
   * @brief Initializes the configuration by reading from the given path.
   *
   * @param path Path to the configuration file
   *
   * @return True if the configuration was successfully initialized otherwise
   * false
   */
  auto init(const std::string& path) -> bool { return read(path); }

  /**
   * @brief Path to the script that is used to start a fabric peer or network
   *
   * @return The path to the start script
   */
  auto start_script_path() -> std::string {
    return config_.get<std::string>("Manager-Fabric.start-script-path");
  }

  /**
   * @brief Path to the test network's directory with a start script,
   * configuration and certificates
   *
   * @return The path to the test network's directory
   */
  auto test_network_dir() -> std::string {
    return config_.get<std::string>("Manager-Fabric.test-network-dir");
  }

  /**
   * @brief When joining a new peer the manager searches for a free port for the
   * peer's communication starting with this port
   *
   * @return The port as a string
   */
  auto starting_peer_port() -> int {
    return config_.get<int>("Manager-Fabric.peer-port");
  }

  /**
   * @brief When joining a new peer the manager searches for a free port for
   * getting the peer's operational metrics starting with this port
   *
   * @return The port as a string
   */
  auto starting_operations_port() -> int {
    return config_.get<int>("Manager-Fabric.operations-port");
  }
};
#endif  // CONFIG_FABRIC_H