/** @defgroup group21 config_stub
 *  @ingroup group2
 *  @{
 */
#ifndef StubAdapter_Stub_Config_H
#define StubAdapter_Stub_Config_H

#include <boost/property_tree/json_parser.hpp>

#include "adapter_interface/adapter_config.h"

/**
 * @brief Define specific configuration values for the stub adapter
 *
 */
class StubConfig : public AdapterConfig {
 public:
  /**
   * @brief Initialize the config bean by parsing the config file
   *
   * @param path Path to file to parse
   * @return true True if parsing was successful
   * @return false False if parsing error occurred, e.g., file not found
   */
  auto init(const std::string &path) -> bool { return read(path); }

  auto set_network_config(const std::string &config) -> bool override {
    // read config
    std::stringstream ss;
    ss << config;
    pt::ptree net_config;
    pt::read_json(ss, net_config);

    // parse network_name
    std::string network_name =
        net_config.get<std::string>("Network.network-name");
    std::string update_datapath = data_path() + network_name;

    config_.put("Adapter-Stub.data-path", update_datapath);
    return true;
  }

  /**
   * @brief Path in which the stub adapter will persist the data
   *
   * @return std::string
   */
  auto data_path() -> std::string {
    return config_.get<std::string>("Adapter-Stub.data-path");
  }

  /**
   * @return number of transactions that make up a block
   */
  auto blocksize() -> int { return config_.get<int>("Adapter-Stub.blocksize"); }
};
#endif  // StubAdapter_Stub_Config_H
/** @} */