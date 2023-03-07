#include "bc_manager_interface_test.h"
#include "manager_stub/manager_stub.h"

#include <unistd.h>

std::string config_path_ = "./test-config.ini";
std::string test_name = "test_stub";
std::string test_network_config_ =
    R"({ "Network": {  "network-name": "test_stub" } })";

/**
 * @brief Reads in a .ini config-file and
 * return the path of the main stub network folder as a string
 *
 * @param path Path to config-file
 *
 * @return The data_path as string
 */
auto read_stub_network_path(const std::string &path) -> std::string {
  boost::property_tree::ptree pt;
  boost::property_tree::read_ini(path, pt);
  return pt.get<std::string>("Manager-Stub.stub-network-path");
}

// Instantiate ManagerInterfaceTest suite
// NOLINTNEXTLINE(modernize-use-trailing-return-type)
INSTANTIATE_TEST_SUITE_P(
    StubManagerTests, BCManagerInterfaceTest,
    testing::Values(std::make_tuple(std::make_shared<StubManager>(),
                                    "./test-config.ini")));

// Single Test-Cases - TEST(test_suite_name, test_name)
TEST(StubManagerTests, Pass) {
  EXPECT_EQ(5, 5);
  EXPECT_TRUE(true);
}

// Check if folder exists
// Check if config file in folder exists
// Check if the number of members matches the number in the config file
// Check if folder is deleted, if last memeber leaves
TEST(StubManagerTests, CreateLeaveNetwork) {
  int members = 1;

  // Create a network
  std::unique_ptr<BcManagerNetworkConfig> network_config_ =
      nullptr; // network configuration
  StubManager s_manager = StubManager();
  s_manager.init(config_path_);
  EXPECT_EQ(s_manager.create_network(test_name, network_config_), 0);

  /****Create a network successful*****/
  // Check if network folder exists
  std::string network_name =
      read_stub_network_path(config_path_) + test_name + "/";
  EXPECT_TRUE(boost::filesystem::is_directory(network_name));

  // Check if config file in folder exists
  std::string config_file_name = network_name + "config.ini";
  EXPECT_TRUE(boost::filesystem::exists(config_file_name));

  // Check if the number of members matches the number in the config file
  std::ifstream ifs;
  ifs.open(config_file_name, std::ios::in);
  std::string str;
  int read_members;
  EXPECT_TRUE(ifs >> str >> read_members);
  ifs.close();
  EXPECT_EQ(read_members, members);

  // Leave the network
  std::unique_ptr<BcManagerNetworkConfig> test_network_config =
      std::make_unique<BcManagerNetworkConfig>();
  test_network_config->read(test_network_config_);
  EXPECT_EQ(s_manager.leave_network(test_name, test_network_config), 0);

  /****Last member successful leaved*****/
  // Check if network folder exists
  EXPECT_TRUE(!boost::filesystem::is_directory(network_name));
}

// Check if folder exists
// Check if config file in folder exists
// Check if the number of members matches the number in the config file
// Check if folder is deleted, if last memeber leaves
TEST(StubManagerTests, JoinLeaveNetwork) {
  int members = 1;

  // Create a network
  std::unique_ptr<BcManagerNetworkConfig> network_config_ =
      nullptr; // network configuration
  StubManager s_manager = StubManager();
  s_manager.init(config_path_);
  EXPECT_EQ(s_manager.create_network(test_name, network_config_), 0);

  // Join the network
  std::unique_ptr<BcManagerNetworkConfig> test_network_config =
      std::make_unique<BcManagerNetworkConfig>();
  test_network_config->read(test_network_config_);
  EXPECT_EQ(s_manager.join_network(test_name, test_network_config), 0);

  /***Join a network successful**/
  // Check if network folder exists
  std::string network_name =
      read_stub_network_path(config_path_) + test_name + "/";
  EXPECT_TRUE(boost::filesystem::is_directory(network_name));

  // Check if config file in folder exists
  std::string config_file_name = network_name + "config.ini";
  EXPECT_TRUE(boost::filesystem::exists(config_file_name));

  // Check if the number of members matches the number in the config file
  std::ifstream ifs;
  ifs.open(config_file_name, std::ios::in);
  std::string str;
  int read_members;
  EXPECT_TRUE(ifs >> str >> read_members);
  EXPECT_EQ(read_members, ++members);
  ifs.close();

  // Leave the network
  EXPECT_EQ(s_manager.leave_network(test_name, test_network_config), 0);

  /****A member successful leaved, but not the last*****/
  // Check if config file still exists
  EXPECT_TRUE(boost::filesystem::exists(config_file_name));

  // Check if the number of members matches the number in the config file
  ifs.open(config_file_name, std::ios::in);
  EXPECT_TRUE(ifs >> str >> read_members);
  ifs.close();
  EXPECT_EQ(read_members, --members);

  // Leave the network
  EXPECT_EQ(s_manager.leave_network(network_name, network_config_), 0);

  /****Last member successful leaved*****/
  // Check if network folder exists
  EXPECT_TRUE(!boost::filesystem::is_directory(network_name));
}