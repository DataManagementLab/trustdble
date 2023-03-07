/** @defgroup group32 config_fabric
 *  @ingroup group3
 *  @{
 */
#ifndef CONFIG_FABRIC_H
#define CONFIG_FABRIC_H

#include "adapter_interface/adapter_config.h"
#include "json.hpp"

/**
 * @brief Define specific configuration values for the Fabric adapter
 *
 */
class FabricConfig : public AdapterConfig {
 public:
  /**
   * @brief Initialize the config bean by parsing the config file
   *
   * @param path Path to file to parse
   * @return true True if parsing was successful
   * @return false False if parsing error occurred, e.g., file not found
   */
  auto init(const std::string& path) -> bool { return read(path); }

  /**
   * @brief Add data from network config to config
   *
   * @param config Json encoded network config
   * @return true True if parsing was successful
   * @return false False if parsing error occurred
   */
  auto set_network_config(const std::string& config) -> bool override {
    nlohmann::json deserialized_config = nlohmann::json::parse(config);
    std::string channel_name = deserialized_config["Network"]["channel_name"];
    config_.put("Adapter-Fabric.channel_name", channel_name);
    std::string peer_port = deserialized_config["Network"]["peer_port"];
    config_.put("Adapter-Fabric.peer_port", peer_port);
    std::string msp_id = deserialized_config["Network"]["msp_id"];
    config_.put("Adapter-Fabric.msp_id", msp_id);
    std::string cert_path = deserialized_config["Network"]["cert_path"];
    config_.put("Adapter-Fabric.cert_path", cert_path);
    std::string key_path = deserialized_config["Network"]["key_path"];
    config_.put("Adapter-Fabric.key_path", key_path);
    std::string tls_cert_path = deserialized_config["Network"]["tls_cert_path"];
    config_.put("Adapter-Fabric.tls_cert_path", tls_cert_path);
    std::string gateway_peer = deserialized_config["Network"]["gateway_peer"];
    config_.put("Adapter-Fabric.gateway_peer", gateway_peer);
    std::string peer_endpoint = deserialized_config["Network"]["peer_endpoint"];
    config_.put("Adapter-Fabric.peer_endpoint", peer_endpoint);
    std::string test_network_path =
        deserialized_config["Network"]["test_network_path"];
    config_.put("Adapter-Fabric.test-network-path", test_network_path);
    return true;
  }

  /**
   * @brief Path to the test network assets
   *
   * @return std::string
   */
  auto test_network_path() -> std::string {
    return config_.get<std::string>("Adapter-Fabric.test-network-path");
  }

  /**
   * @brief Path to trustdble-adapters folder
   *
   * @return std::string
   */
  auto adapters_path() -> std::string {
    return config_.get<std::string>("Adapter-Fabric.adapters-path");
  }

  /**
   * @brief Name of fabric network's channel
   *
   * @return std::string
   */
  auto channel_name() -> std::string {
    return config_.get<std::string>("Adapter-Fabric.channel_name");
  }

  /**
   * @brief Port of the peer that was added to the fabric network for
   * this instance
   *
   * @return std::string
   */
  auto peer_port() -> std::string {
    return config_.get<std::string>("Adapter-Fabric.peer_port");
  }

  /**
   * @brief The id of the membership service provider
   *
   * @return The msp id as a string
   */
  auto msp_id() -> std::string {
    return config_.get<std::string>("Adapter-Fabric.msp_id");
  }

  /**
   * @brief The path to the certificate of the client identity
   * used to transact with the network
   *
   * @return The path as a string
   */
  auto cert_path() -> std::string {
    return config_.get<std::string>("Adapter-Fabric.cert_path");
  }

  /**
   * @brief The path to a directoy with private keys of the client
   * used to transact with the network
   *
   * @return The path as a string
   */
  auto key_path() -> std::string {
    return config_.get<std::string>("Adapter-Fabric.key_path");
  }

  /**
   * @brief The path to the gateway peers tls certificate
   *
   * @return The path as a string
   */
  auto tls_cert_path() -> std::string {
    return config_.get<std::string>("Adapter-Fabric.tls_cert_path");
  }

  /**
   * @brief The gateway peer's name
   *
   * @return The name as a string
   */
  auto gateway_peer() -> std::string {
    return config_.get<std::string>("Adapter-Fabric.gateway_peer");
  }

  /**
   * @brief The address where the gateway peer can be reached
   *
   * @return The address as a string
   */
  auto peer_endpoint() -> std::string {
    return config_.get<std::string>("Adapter-Fabric.peer_endpoint");
  }
};
#endif  // CONFIG_FABRIC_H
/** @} */