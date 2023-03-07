#ifndef BC_MANAGER_INTERFACE_TEST_H
#define BC_MANAGER_INTERFACE_TEST_H

#include <gtest/gtest.h>
#include <sys/stat.h>

#include <array>
#include <filesystem>

#include "bc_manager_interface/bc_manager_interface.h"

// Test-Fixture for bc_manager base tests. Tests are implemented in
// bc_manager_interface_test.cpp

/**
 * @brief Test fixture for generic bc_manager tests. All bc_manager tests should
 * implement it.
 *
 * This [text
 * fixture](https://github.com/google/googletest/blob/master/docs/primer.md#test-fixtures-using-the-same-data-configuration-for-multiple-tests-same-data-multiple-tests)
 * provides generic setup and teardown logic that all bc_manager tests can use.
 * It implements generic tests that all new bc_managers should bass in the
 * corresponding .cpp file
 *
 */
class BCManagerInterfaceTest
    : public testing::TestWithParam<
          std::tuple<std::shared_ptr<BcManager>, std::string>> {
 protected:
  /**
   * @brief Initialize any required variables and objects for the test
   *
   */
  void SetUp() override {
    // called before each test
    manager_ = std::get<0>(GetParam());
    config_path_ = std::get<1>(GetParam());

    // initialize manager with configuration
    manager_->init(config_path_);
  }

  /**
   * @brief Clean up after each test
   *
   */
  void TearDown() override {
    manager_->shutdown();  // TODO not implemented for ETH
    manager_.reset();
  }

  // BcManager instance that is used for the tests
  std::shared_ptr<BcManager> manager_;
  // path to the configuration file that the BcManager uses
  std::string config_path_;

  // network name that is used for the tests
  std::string network_name_ = "test_network";
  std::string network_name_2_ = "test_network_2";
};
#endif  // BC_MANAGER_INTERFACE_TEST_H
