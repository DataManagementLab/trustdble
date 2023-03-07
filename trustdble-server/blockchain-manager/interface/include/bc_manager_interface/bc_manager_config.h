#ifndef BC_MANAGER_CONFIG_H
#define BC_MANAGER_CONFIG_H

#include <boost/property_tree/ini_parser.hpp>
#include <boost/property_tree/ptree.hpp>

namespace pt = boost::property_tree;

/**
 * @brief The Manager Configuration is used to specify variables like paths for example that a specific manager needs to work properly.
 * When initializing a specific manager you have to provide a path to a fitting configuration as an input.
 *
 */
class BcManagerConfig {
 public:
    virtual ~BcManagerConfig() = default;
 protected:
  pt::ptree config_;

  /**
   * Reads in a .ini config-file and initialize the config
   *
   * @param path Path to config-file
   */
  auto read(const std::string &path) -> bool {
    pt::read_ini(path, config_);
    return true;
  }
};

#endif  // BC_MANAGER_CONFIG_H