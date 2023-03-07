#ifndef BC_MANAGER_INTERFACE_H
#define BC_MANAGER_INTERFACE_H

#include <string>
#include <vector>

#include "bc_manager_interface/bc_manager_networkconfig.h"

/**
 * @brief Interface definition to be used to deploy a blockchain network
 *
 */
class BcManager {
 public:
  virtual ~BcManager() = default;

  ///////////////// Interface methods //////////////////////

  /**
   * @brief Initialize manager with configuration
   *
   * @param[in] config_path Path to configuration
   *
   * @return True if init was successful, otherwise false
   */
  virtual auto init(const std::string &config_path) -> bool = 0;

  /**
   * @brief Uninitialize manager
   *
   * @return True if shutdown was successful, otherwise false
   */
  virtual auto shutdown() -> bool = 0;

  /**
   * @brief Creates a new network and provides its network configuration.
   *
   * @param[in] name The name of the network's first node (how do we map this to
   * fabric?)
   * @param[out] config The network configuration as out parameter
   * @return int returns 0 on success, 1 on failure
   */
  virtual auto create_network(const std::string &name,
                              std::unique_ptr<BcManagerNetworkConfig> &config)
      -> int = 0;

  /**
   * @brief Joins an existing network by using an existing joinable network
   * configuration. That means it does not have to be a full configuration. It
   * only needs to contain all necessary parameters for joining a network that
   * depends on the network type. Local parameters are not needed. They will be
   * updated after joining the network.
   *
   * @param[in] name The name of the new node that joins the network
   * @param[in,out] config The joinable network configuration that is updated
   * with local parameters.
   * @return int returns 0 on success, 1 on failure
   */
  virtual auto join_network(const std::string &name,
                            std::unique_ptr<BcManagerNetworkConfig> &config)
      -> int = 0;

  /**
   * @brief Leave a running network node
   *
   * @param[in] name The name of the network
   * @param[in] config The configuration of the network
   * @return int returns 0 on success, 1 on failure
   */
  virtual auto leave_network(const std::string &name,
                             std::unique_ptr<BcManagerNetworkConfig> &config)
      -> int = 0;
};

#endif  // BC_MANAGER_INTERFACE_H