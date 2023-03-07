#ifndef CONFIG_STUB_H
#define CONFIG_STUB_H

#include "bc_manager_interface/bc_manager_config.h"
#include <boost/filesystem.hpp>
#include <iostream>

class StubManagerConfig : public BcManagerConfig {
 public:
  explicit StubManagerConfig() = default;
  ~StubManagerConfig() override = default;

  /**
   * @brief Initializes the configuration by reading from the given path.
   *
   * @param path Path to the configuration file
   *
   * @return True if the configuration was successfully initialized otherwise false
   */
  auto init(const std::string& path) -> bool { 
    if (!read(path)) {
      return false; // read .ini failed
    } 

    std::string stub_network_path = config_.get<std::string>("Manager-Stub.stub-network-path");

    // get datapath failed or folder not exists
    return !(stub_network_path.empty() || !boost::filesystem::is_directory(stub_network_path));
    }
  
  /**
   * @brief The path of the main stub network folder. 
   * All stub networks should be saved inside this folder.
   *
   * @return The data_path as string
   */
  auto stub_network_path() -> std::string {
    return config_.get<std::string>("Manager-Stub.stub-network-path");
    } 

};
#endif  // CONFIG_STUB_H