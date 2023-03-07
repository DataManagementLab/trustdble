#include <boost/property_tree/json_parser.hpp>
#include <boost/property_tree/ptree.hpp>

#include "bc_manager_interface_test.h"
#include "manager_ethereum/manager_ethereum.h"

// size for buffer to get return after executing node command
#define BUFFER_SIZE_EXEC 128

std::string network_name = "ethereum_network";
std::string container_name = "ethereum-node";
std::string config_path = "./test-config.ini";
int rpc_port;
int peer_port;

/**
 * @brief Read the rpc and peer port from the given ini file with the help of
 * ptree.
 */
static auto read_port_from_config() {
  boost::property_tree::ptree pt;
  boost::property_tree::read_ini(config_path, pt);
  rpc_port = pt.get<int>("Manager-Ethereum.rpc-port");
  peer_port = pt.get<int>("Manager-Ethereum.peer-port");
}

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

  auto *pipe = popen(cmd.c_str(), "r"); // execute command and capture stdout
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
INSTANTIATE_TEST_SUITE_P(
    EthereumManagerTests, BCManagerInterfaceTest,
    testing::Values(std::make_tuple(std::make_shared<EthereumManager>(),
                                    "./test-config.ini")));

// Single Test-Cases - TEST(test_suite_name, test_name)
// NOLINTNEXTLINE(modernize-use-trailing-return-type)
TEST(EthereumManagerTests, Pass) {
  EXPECT_EQ(5, 5);
  EXPECT_TRUE(true);
}

// Check if containers are started
// Check for correct container name (including network name and name from
// config) Check if container ports match the ports from config Check if
// container are stopped when leaving a network
// NOLINTNEXTLINE(modernize-use-trailing-return-type)
TEST(EthereumManagerTests, Container) {
  // Create a network
  std::unique_ptr<BcManagerNetworkConfig> network_config = nullptr;
  EthereumManager e_manager = EthereumManager();
  e_manager.init(config_path);
  EXPECT_EQ(e_manager.create_network(network_name, network_config), 0);

  /****Network successful created*****/
  // get the info from cmd: docker inspect container_name and set it into ptree
  boost::property_tree::ptree pt =
      get_container_info((container_name + "_" + network_name).c_str());

  // Check if containers are started and for correct container name
  // the ptree must not be null
  EXPECT_TRUE(!pt.empty());
  // the ptree must have a node with the right name
  EXPECT_EQ(pt.get<std::string>("Name"),
            "/" + container_name + "_" + network_name);
  // the container status must be running
  EXPECT_EQ(pt.get<std::string>("State.Status"), "running");

  // Check if container ports match the ports from config
  read_port_from_config(); // read the port info and update the variable
                           // peer/rpc_port
  for (auto &item : pt.get_child("HostConfig.PortBindings." +
                                 std::to_string(peer_port) + "/tcp")) {
    EXPECT_EQ(item.second.get<int>("HostPort"), peer_port);
  }

  for (auto &item : pt.get_child("HostConfig.PortBindings." +
                                 std::to_string(rpc_port) + "/tcp")) {
    EXPECT_EQ(item.second.get<int>("HostPort"), rpc_port);
  }

  // Leave the network
  EXPECT_EQ(e_manager.leave_network(network_name, network_config), 0);

  /****Network successful leaved*****/
  // Check if container are removed when leaving a network
  pt = get_container_info((container_name + "_" + network_name).c_str());
  EXPECT_TRUE(pt.empty());
}
