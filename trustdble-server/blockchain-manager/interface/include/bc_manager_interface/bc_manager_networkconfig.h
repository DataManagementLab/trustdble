#ifndef BC_MANAGER_NETWORKCONFIG_H
#define BC_MANAGER_NETWORKCONFIG_H

#include <boost/property_tree/json_parser.hpp>
#include <boost/property_tree/ptree.hpp>

namespace pt = boost::property_tree;

/**
 * @brief The Network Configuration stores all parameters of a network including the type of the network and its id.
 * Blockchain-specific parameters can be accessed by casting it to a specific Blockchain Network Configuration.
 * It is returned when a network is created.
 * When joining a network you have to provide this configuration as an input.
 */
class BcManagerNetworkConfig {
 public:
  virtual ~BcManagerNetworkConfig() = default;

 protected:
  pt::ptree config_;

 public:

  /**
   * @brief The ID of this network.
   *
   * @return The Network-ID
   */
  auto id() -> std::string {
    return config_.get<std::string>("Network.id");
  }

  /**
   * @brief Sets the ID of this network configuration.
   * @param id ID of the network
   */
  auto set_id(std::string id) {
    config_.put("Network.id", id);
  }


  /**
   * @brief Reads in a JSON string and initialize the config
   *
   * @param config Config as JSON string
   *
   * @return TRUE if reading config was successful otherwise FALSE.
   */
  auto read(const std::string &config) -> bool {
    std::stringstream ss;
    ss << config;
    pt::read_json(ss, config_);
    return true;
  }

  /**
   * @brief Writes out the configuration as a JSON string
   *
   * @return Config as JSON string
   */
  auto get_json() -> std::string {
    std::string json_string;
    std::stringstream ss;
    pt::write_json(ss, config_);
    json_string = ss.str();
    std::replace( json_string.begin(), json_string.end(), '\n', ' ');
    return json_string;
  }
};

#endif  // BC_MANAGER_NETWORKCONFIG_H