#include "bc_manager_interface_test.h"

#include "json.hpp"

// define generic tests below
/**
 * @file
 * @brief This file contains generic test cases, that all bc_managers
 * implementations should pass.
 *
 */

/**
 * @brief Test that create network provides valid network configuration
 * and corresponding container is created and running,
 * Test that leave network stops and removes corresponding container
 *
 */
// NOLINTNEXTLINE(modernize-use-trailing-return-type)
TEST_P(BCManagerInterfaceTest /*unused*/, CreateAndLeaveNetwork /*unused*/) {
  // create network
  std::unique_ptr<BcManagerNetworkConfig> network_config =
      nullptr;  // network configuration
  EXPECT_EQ(manager_->create_network(network_name_, network_config), 0);

  // verify network configuration
  std::string network_config_json_str = network_config->get_json();
  EXPECT_FALSE(network_config_json_str.empty());  // check is not empty
  EXPECT_TRUE(nlohmann::json::accept(
      network_config_json_str));  // check structure = json

  // leave network
  EXPECT_EQ(manager_->leave_network(network_name_, network_config), 0);
}

/**
 * @brief Test create a network, use the network config
 * to start a second container and join the network,
 * leave the network with the second container, check that the container is
 * removed
 *
 */
// NOLINTNEXTLINE(modernize-use-trailing-return-type)
TEST_P(BCManagerInterfaceTest /*unused*/, JoinAndLeaveNetwork /*unused*/) {
  // create network A
  std::unique_ptr<BcManagerNetworkConfig> network_config =
      nullptr;  // network configuration
  EXPECT_EQ(manager_->create_network(network_name_, network_config), 0);

  // join network A from network B
  std::string network_name_join = network_name_ + "_join";  // network name join
  EXPECT_EQ(manager_->join_network(network_name_join, network_config), 0);

  // leave network by B side
  EXPECT_EQ(manager_->leave_network(network_name_join, network_config), 0);

  // leave network by A side
  EXPECT_EQ(manager_->leave_network(network_name_, network_config), 0);
}

/**
 * @brief Test create two networks running in paralell,
 * leave the networks
 *
 */
// NOLINTNEXTLINE(modernize-use-trailing-return-type)
TEST_P(BCManagerInterfaceTest /*unused*/,
       createAndLeaveTwoNetworks /*unused*/) {
  // create first network
  std::unique_ptr<BcManagerNetworkConfig> network_config_1 =
      nullptr;  // network configuration
  EXPECT_EQ(manager_->create_network(network_name_, network_config_1), 0);

  // create second network
  std::unique_ptr<BcManagerNetworkConfig> network_config_2 =
      nullptr;  // network configuration
  EXPECT_EQ(manager_->create_network(network_name_2_, network_config_2), 0);

  // leave first network
  EXPECT_EQ(manager_->leave_network(network_name_, network_config_1), 0);

  // leave second network
  EXPECT_EQ(manager_->leave_network(network_name_2_, network_config_2), 0);
}

/**
 * @brief Create a network and leave the network, thus deleting the network
 * Try joining the deleted network. This should fail
 *
 */
// NOLINTNEXTLINE(modernize-use-trailing-return-type)
TEST_P(BCManagerInterfaceTest /*unused*/, joinAfterLeavingNetwork /*unused*/) {
  // WIth only one network this test would be trivial for fabric, because
  // leaving the network whould result in the whole test network being removed.
  std::unique_ptr<BcManagerNetworkConfig> network_config_1 =
      nullptr;  // network configuration
  EXPECT_EQ(manager_->create_network(network_name_, network_config_1), 0);

  // create second network
  std::unique_ptr<BcManagerNetworkConfig> network_config_2 =
      nullptr;  // network configuration
  EXPECT_EQ(manager_->create_network(network_name_2_, network_config_2), 0);

  // leave second network
  EXPECT_EQ(manager_->leave_network(network_name_2_, network_config_2), 0);

  // try joining the second network. this should fail
  EXPECT_EQ(manager_->join_network(network_name_2_, network_config_2), 1);

  // leave first network
  EXPECT_EQ(manager_->leave_network(network_name_, network_config_1), 0);
}
