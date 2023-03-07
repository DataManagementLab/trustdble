/** @defgroup group11 config_ethereum
 *  @ingroup group1
 *  @{
 */
#ifndef CONFIG_ETHEREUM_H
#define CONFIG_ETHEREUM_H

#include <boost/property_tree/json_parser.hpp>

#include "adapter_interface/adapter_config.h"

/**
 * @brief Define specific configuration values for the Ethereum adapter
 *
 */
class EthereumConfig : public AdapterConfig {
 public:
  /**
   * @brief Initialize the config bean by parsing the config file
   *
   * @param path Path to file to parse
   * @return true if parsing was successful otherwise false
   */
  auto init_path(const std::string& path) -> bool { return read(path); }

  /**
   * @brief Initialize the config by parsing a json string from blockchain
   * manager
   *
   * @param config JSON formatted string to parse
   * @return true if parsing was successful otherwise false
   */
  auto set_network_config(const std::string& config) -> bool override {
    // read config
    std::stringstream ss;
    ss << config;
    pt::ptree net_config;
    pt::read_json(ss, net_config);

    // parse join_ip
    std::string join_ip = net_config.get<std::string>("Network.join-ip");
    // parse rpc_port
    std::string rpc_port = net_config.get<std::string>("Network.rpc-port");
    // create connection_url = "http://" + join_ip + ":" + rpc_port
    std::string connection_url = "http://" + join_ip + ":" + rpc_port;

    // set connection_url in adapter config
    config_.put("Adapter-Ethereum.connection-url", connection_url);
    return true;
  }

  /**
   * @brief The IP-Address of the Ethereum (geth) node to connect to
   *
   * @return std::string
   */
  auto connection_url() -> std::string {
    return config_.get<std::string>("Adapter-Ethereum.connection-url");
  }

  /**
   * @brief The address of (table) contract that the adapter will use for
   * reads/writes
   *
   * @return std::string
   */
  auto contract_address() -> std::string {
    return config_.get<std::string>("Adapter-Ethereum.contract-address");
  }

  /**
   * @brief The path to the compiled contract file that the adapter will use for
   * deploying the contract to the blockchain
   *
   * @return std::string
   */
  auto contract_path() -> std::string {
    return config_.get<std::string>("Adapter-Ethereum.contract-path");
  }

  /**
   * @brief The maximum time the adapter will wait for a blockchain TX to be
   * mined
   *
   * @return int
   */
  auto max_waiting_time() -> int {
    return config_.get<int>("Adapter-Ethereum.max-waiting-time");
  }

  /**
   * @brief Path to the folder containing the scripts to deploy contracts etc.
   *
   * @return std::string
   */
  auto script_path() -> std::string {
    return config_.get<std::string>("Adapter-Ethereum.script-path");
  }
};
#endif  // CONFIG_ETHEREUM_H
/** @} */
