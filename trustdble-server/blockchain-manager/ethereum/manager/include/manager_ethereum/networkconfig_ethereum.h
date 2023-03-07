#ifndef NETWORKCONFIG_ETHEREUM_H
#define NETWORKCONFIG_ETHEREUM_H

#include "bc_manager_interface/bc_manager_networkconfig.h"

class EthereumNetworkConfig : public BcManagerNetworkConfig {
 public:
  explicit EthereumNetworkConfig() = default;
  ~EthereumNetworkConfig() = default;

  /**
   * @brief Initializes the configuration with the provided parameters.
   *
   * @param rpc_port The RPC port that the Ethereum node is running on. It is used for the Rest-API endpoint.
   *
   * @param peer_port The peer port that the Ethereum node is running on. It is used for peer communication between ethereum nodes.
   *
   * @param join_ip The ip of the current system on that other nodes can join. It is added to the network id when creating a network.
   *
   * @return True if the configuration was successfully initialized otherwise false
   */
  auto init(const int rpc_port, const int peer_port, const std::string& join_ip) -> bool {
    config_.put("Network.rpc-port",rpc_port);
    config_.put("Network.peer-port",peer_port);
    config_.put("Network.join-ip",join_ip);

    return true;
   }

  /**
   * @brief The RPC port that the Ethereum node is running on. It is used for the Rest-API endpoint.
   *
   * @return The RPC Port as integer
   */
  auto rpc_port() -> int {
    return config_.get<int>("Network.rpc-port");
  }

  /**
   * @brief The peer port that the Ethereum node is running on. It is used for peer communication between ethereum nodes.
   *
   * @return The peer port as integer
   */
  auto peer_port() -> int {
    return config_.get<int>("Network.peer-port");
  }

  /**
   * @brief The ip of the current system on that other nodes can join. It is added to the network id when creating a network.
   *
   * @return The join ip as string
   */
  auto join_ip() -> std::string {
    return config_.get<std::string>("Network.join-ip");
  }

  /**
   * @brief The enode string of the network node on that other nodes can join.
   *
   * @return The enode string
   */
  auto enode() -> std::string {
    return config_.get<std::string>("Network.enode");
  }

  /**
   * @brief Sets the enode string of this network configuration.
   *
   * @param enode The enode string
   */
  auto set_enode(std::string enode) {
    config_.put("Network.enode", enode);
  }
};
#endif  // NETWORKCONFIG_ETHEREUM_H