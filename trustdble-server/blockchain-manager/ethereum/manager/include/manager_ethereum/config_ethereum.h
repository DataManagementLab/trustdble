#ifndef CONFIG_ETHEREUM_H
#define CONFIG_ETHEREUM_H

#include "bc_manager_interface/bc_manager_config.h"

class EthereumConfig : public BcManagerConfig {
 public:
  explicit EthereumConfig() = default;
  ~EthereumConfig() override = default;

  /**
   * @brief Initializes the configuration by reading from the given path.
   *
   * @param path Path to the configuration file
   *
   * @return True if the configuration was successfully initialized otherwise false
   */
  auto init(const std::string& path) -> bool { return read(path); }

  /**
   * @brief The RPC port that the Ethereum node is running on. It is used for the Rest-API endpoint.
   *
   * @return The RPC Port as integer
   */
  auto rpc_port() -> int {
    return config_.get<int>("Manager-Ethereum.rpc-port");
  }

   /**
   * @brief The peer port that the Ethereum node is running on. It is used for peer communication between ethereum nodes.
   *
   * @return The peer port as integer
   */
  auto peer_port() -> int {
    return config_.get<int>("Manager-Ethereum.peer-port");
  }

   /**
   * @brief The ip of the current system on that other nodes can join. It is added to the network id when creating a network.
   *
   * @return The join ip as string
   */
  auto join_ip() -> std::string {
    return config_.get<std::string>("Manager-Ethereum.join-ip");
  }

  /**
   * @brief Path to the script that is used to start a ethereum node.
   *
   * @return The path to the script
   */
  auto start_script_path() -> std::string {
    return config_.get<std::string>("Manager-Ethereum.start-script-path");
  }

  /**
   * @brief Path to the docker folder containing all necessary files that are used to create the ethereum docker image.
   *
   * @return The path to the docker folder
   */
  auto docker_file_path() -> std::string {
    return config_.get<std::string>("Manager-Ethereum.docker-file-path");
  }

};
#endif  // CONFIG_ETHEREUM_H