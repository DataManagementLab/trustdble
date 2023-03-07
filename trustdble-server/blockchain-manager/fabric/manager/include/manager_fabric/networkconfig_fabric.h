#ifndef NETWORKCONFIG_FABRIC_H
#define NETWORKCONFIG_FABRIC_H

#include "bc_manager_interface/bc_manager_networkconfig.h"

class FabricNetworkConfig : public BcManagerNetworkConfig {
 public:
  explicit FabricNetworkConfig() = default;
  ~FabricNetworkConfig() override = default;

  /**
   * @brief Initializes the configuration.
   *
   * @return True if the configuration was successfully initialized otherwise
   * false
   */
  auto init(const std::string& channel_name, const std::string& msp_id,
            const std::string& cert_path, const std::string& key_path,
            const std::string& tls_cert_path, const std::string& gateway_peer,
            const std::string& test_network_path) -> bool {
    config_.put("Network.channel_name", channel_name);
    config_.put("Network.msp_id", msp_id);
    config_.put("Network.cert_path", cert_path);
    config_.put("Network.key_path", key_path);
    config_.put("Network.tls_cert_path", tls_cert_path);
    config_.put("Network.gateway_peer", gateway_peer);
    config_.put("Network.test_network_path", test_network_path);
    return true;
  }

  /**
   * @brief The name of the network
   *
   * @return The network name as a string
   */
  auto channel_name() -> std::string {
    return config_.get<std::string>("Network.channel_name");
  }

  /**
   * @brief The id of the membership service provider
   *
   * @return The msp id as a string
   */
  auto msp_id() -> std::string {
    return config_.get<std::string>("Network.msp_id");
  }

  /**
   * @brief The path to the certificate of the client identity
   * used to transact with the network
   *
   * @return The path as a string
   */
  auto cert_path() -> std::string {
    return config_.get<std::string>("Network.cert_path");
  }

  /**
   * @brief The path to a directoy with private keys of the client
   * used to transact with the network
   *
   * @return The path as a string
   */
  auto key_path() -> std::string {
    return config_.get<std::string>("Network.key_path");
  }

  /**
   * @brief The path to the gateway peers tls certificate
   *
   * @return The path as a string
   */
  auto tls_cert_path() -> std::string {
    return config_.get<std::string>("Network.tls_cert_path");
  }

  /**
   * @brief The gateway peer's name
   *
   * @return The name as a string
   */
  auto gateway_peer() -> std::string {
    return config_.get<std::string>("Network.gateway_name");
  }

  /**
   * @brief The peer port that the new peer is running on. It is used for
   * communication between peers
   *
   * @return The peer port as an integer
   */
  auto peer_port() -> int { return config_.get<int>("Network.peer_port"); }

  auto set_peer_port(const int peer_port) -> bool {
    config_.put<int>("Network.peer_port", peer_port);
    return true;
  }

  /**
   * @brief The operations port for getting operational metrics for monitoring
   * from the peer
   *
   * @return The operations port as an integer
   */
  auto operations_port() -> int {
    return config_.get<int>("Network.peer_operations_port");
  }

  auto set_operations_port(const int operations_port) -> bool {
    config_.put<int>("Network.peer_operations_port", operations_port);
    return true;
  }

  /**
   * @brief The address where the gateway peer can be reached
   *
   * @return The address as a string
   */
  auto peer_endpoint() -> std::string {
    return config_.get<std::string>("Network.peer_endpoint");
  }

  auto set_peer_endpoint(const std::string& peer_endpoint) -> bool {
    config_.put<std::string>("Network.peer_endpoint", peer_endpoint);
    return true;
  }
};
#endif  // NETWORKCONFIG_FABRIC_H