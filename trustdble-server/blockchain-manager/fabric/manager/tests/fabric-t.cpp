#include <boost/property_tree/json_parser.hpp>
#include <boost/property_tree/ptree.hpp>

#include "bc_manager_interface_test.h"
#include "manager_fabric/manager_fabric.h"

// size for buffer to get return after executing node command
#define BUFFER_SIZE_EXEC 128

std::string config_path = "./test-config.ini";
std::string create_network_name = "added-peer-1";
std::string join_network_name = "added-peer-2";

/**
 * @brief Execute system docker inspect container_name commands,
 * and parse the output info to a ptree, and return the ptree.
 *
 * @param name The name of container to look for.
 *
 * @return ptree
 */
static auto get_container_info(const char *name)
    -> boost::property_tree::ptree {
  boost::property_tree::ptree pt;

  std::string cmd = "docker inspect " + std::string{name};

  auto *pipe = popen(cmd.c_str(), "r");  // execute command and capture stdout
  // this only checks if capturing the stdoutput of the command was successful
  // it does not say anything about the success of the executed command (ie. its
  // return code)
  if (pipe == nullptr) {
    throw std::runtime_error("popen() failed!");
  }

  // read the captured output and store it in a string
  std::array<char, BUFFER_SIZE_EXEC> buffer;
  std::string result;
  while (feof(pipe) == 0) {
    if (fgets(buffer.data(), buffer.size(), pipe) != nullptr) {
      result += buffer.data();
    }
  }

  int return_code = pclose(pipe);
  // command execution was not successful => return a empty ptree
  if (return_code != EXIT_SUCCESS) {
    return pt;
  }

  // command execution was successful

  // the output is as a array of ptree, but with only a element
  // so remove the first "[" and last "]" and parse the result to ptree
  int first_posi = result.find_first_of('[');
  int last_posi = result.find_last_of(']');
  result.erase(last_posi, 1);
  result.erase(first_posi, 1);

  std::stringstream ss;
  ss << result;
  boost::property_tree::read_json(ss, pt);

  return pt;
}

// Instantiate ManagerInterfaceTest suite
// NOLINTNEXTLINE(modernize-use-trailing-return-type)
INSTANTIATE_TEST_SUITE_P(FabricManagerTests, BCManagerInterfaceTest,
                         testing::Values(std::make_tuple(
                             std::make_shared<FabricManager>(), config_path)));

// Create a network and then join another peer to it
// Check if containers are started
// Check for correct container name (including network name and name from
// config) Check if container ports match the ports from config Check if
// containers are stopped when leaving a network
// NOLINTNEXTLINE(modernize-use-trailing-return-type)
TEST(FabricManagerTests, Container) {
  // Create a network
  std::unique_ptr<BcManagerNetworkConfig> config = nullptr;
  FabricManager f_manager = FabricManager();
  f_manager.init(config_path);
  EXPECT_EQ(f_manager.create_network(create_network_name, config), 0);

  /****Network created successfully*****/
  // get the info from cmd: docker inspect contaniername and set it into ptree
  FabricNetworkConfig *network_config = (FabricNetworkConfig *)config.get();
  boost::property_tree::ptree pt =
      get_container_info((create_network_name).c_str());

  // Check if container was started and for correct container name
  // the ptree must not be null
  EXPECT_TRUE(!pt.empty());
  // the ptree must have a node with the right name
  EXPECT_EQ(pt.get<std::string>("Name"), "/" + create_network_name);
  // the container status must be running
  EXPECT_EQ(pt.get<std::string>("State.Status"), "running");

  // Check if container ports match the ports from config
  for (auto &item :
       pt.get_child("HostConfig.PortBindings." +
                    std::to_string(network_config->peer_port()) + "/tcp")) {
    EXPECT_EQ(item.second.get<int>("HostPort"), network_config->peer_port());
  }

  for (auto &item : pt.get_child(
           "HostConfig.PortBindings." +
           std::to_string(network_config->operations_port()) + "/tcp")) {
    EXPECT_EQ(item.second.get<int>("HostPort"),
              network_config->operations_port());
  }

  // Join another peer to the network
  EXPECT_EQ(f_manager.join_network(join_network_name, config), 0);

  /****Network joined successfully*****/
  // get the info from cmd: docker inspect contaniername and set it into ptree
  pt = get_container_info((join_network_name).c_str());

  // Check if container was started and for correct container name
  // the ptree must not be null
  EXPECT_TRUE(!pt.empty());
  // the ptree must have a node with the right name
  EXPECT_EQ(pt.get<std::string>("Name"), "/" + join_network_name);
  // the container status must be running
  EXPECT_EQ(pt.get<std::string>("State.Status"), "running");

  // Check if container ports match the ports from config
  for (auto &item :
       pt.get_child("HostConfig.PortBindings." +
                    std::to_string(network_config->peer_port()) + "/tcp")) {
    EXPECT_EQ(item.second.get<int>("HostPort"), network_config->peer_port());
  }

  for (auto &item : pt.get_child(
           "HostConfig.PortBindings." +
           std::to_string(network_config->operations_port()) + "/tcp")) {
    EXPECT_EQ(item.second.get<int>("HostPort"),
              network_config->operations_port());
  }

  // Leave the network
  EXPECT_EQ(f_manager.leave_network(join_network_name, config), 0);
  EXPECT_EQ(f_manager.leave_network(create_network_name, config), 0);

  /****Network left successfully*****/
  // Check if containers are removed when leaving a network
  pt = get_container_info((create_network_name).c_str());
  EXPECT_TRUE(pt.empty());
  pt = get_container_info((join_network_name).c_str());
  EXPECT_TRUE(pt.empty());
}
