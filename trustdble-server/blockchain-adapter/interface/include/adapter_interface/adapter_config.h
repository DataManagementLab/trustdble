/** @defgroup group51 adapter_config
 *  @ingroup group5
 *  @{
 */
#ifndef ADAPTER_CONFIG_H
#define ADAPTER_CONFIG_H

#include <boost/property_tree/ini_parser.hpp>
#include <boost/property_tree/ptree.hpp>

namespace pt = boost::property_tree;

/**
 * @brief Generic Config class for adapter interface
 *
 */
class AdapterConfig {
 protected:
  //! Property tree in which config key-value-pairs are stored
  pt::ptree config_;

  /**
   * @brief Parse a config file into a property tree
   *
   * @param path Path to the config file to parse
   * @return true If parsing was successful otherwise false
   */
  auto read(const std::string &path) -> bool {
    pt::read_ini(path, config_);
    return true;
  }

  /**
   * @brief Set the network configuration
   *
   * @param config Network configuration as JSON formatted string from
   * Blockchain Manager
   * @return true if successfull otherwise false
   */
  virtual auto set_network_config(const std::string &config) -> bool = 0;
};

#endif  // ADAPTER_CONFIG_H
/** @} */