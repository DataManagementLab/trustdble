#include "manager_ethereum/manager_ethereum.h"

#include <sys/time.h>
#include <unistd.h>

#include <boost/asio.hpp>

#include "manager_utils/shell_helpers.h"

#define TEN_SEC 10000000     // 10 sec in microseconds
#define ONE_MIN 60000000     // 60 sec in microseconds
#define THREE_MIN 180000000  // 180 sec in microseconds

// Constants
const std::string kContainerName = "ethereum-node";

///////// EthereumManager Implementation //////////////////////

// Constructor
EthereumManager::EthereumManager() = default;

// Destructor
EthereumManager::~EthereumManager() = default;

auto EthereumManager::init(const std::string &config_path) -> bool {
  // Init Ethereum config
  BOOST_LOG_TRIVIAL(debug) << "EthereumManager: Init, config-path="
                           << config_path;
  config_.init(config_path);

  return true;
}

auto EthereumManager::shutdown() -> bool { return true; }

auto EthereumManager::create_network(
    const std::string &name, std::unique_ptr<BcManagerNetworkConfig> &config)
    -> int {
  unsigned short rpc_port = config_.rpc_port();
  unsigned short peer_port = config_.peer_port();
  std::string join_ip = config_.join_ip();

  while (port_in_use(rpc_port)) {
    rpc_port++;
  }
  while (port_in_use(peer_port)) {
    peer_port++;
  }

  std::unique_ptr<EthereumNetworkConfig> network_config =
      std::make_unique<EthereumNetworkConfig>();
  network_config->init(rpc_port, peer_port, join_ip);

  std::string start_cmd = "bash " + config_.start_script_path() + " " +
                          kContainerName + "_" + name + " " +
                          std::to_string(network_config->rpc_port()) + " " +
                          std::to_string(network_config->peer_port()) + " " +
                          config_.docker_file_path();

  if (system(start_cmd.c_str()) != 0) {
    BOOST_LOG_TRIVIAL(debug)
        << "EthereumManager: create_network, start ethereum node failed!";
    return 1;
  }

  // Wait until DAG generation process is done
  BOOST_LOG_TRIVIAL(debug)
      << "EthereumManager: create_network, waiting for DAG generation...";
  std::string wait_dag_cmd = "docker logs -f " + kContainerName + "_" + name +
                             " | sed -n '/Successfully sealed new block/q'";

  bool done;
  retry_exec(wait_dag_cmd.c_str(), &done, THREE_MIN);
  if (!done) {
    std::string error_msg =
        "TIMEOUT during execution of the following command:\n";
    error_msg = error_msg + wait_dag_cmd;
    BOOST_LOG_TRIVIAL(debug) << error_msg;
    return 1;
  }
  BOOST_LOG_TRIVIAL(debug)
      << "EthereumManager: create_network, DAG generation done!";

  // Fetch enode
  std::string enode_cmd = "docker exec -it " + kContainerName + "_" + name +
                          " /bin/bash -c  \"geth --exec admin.nodeInfo.enode "
                          "attach eth-data/geth.ipc \"";

  bool enode_cmd_successful;
  std::string raw_enode =
      retry_exec(enode_cmd.c_str(), &enode_cmd_successful, TEN_SEC);

  if (!enode_cmd_successful) {  // time out
    std::string error_msg =
        "TIMEOUT during execution of the following command:\n";
    error_msg = error_msg + enode_cmd + "\nReturn code was: " + raw_enode;
    BOOST_LOG_TRIVIAL(debug) << error_msg;
    return 1;
  }

  const size_t start_pos = 6;
  const size_t len_enode = raw_enode.find('@') - 5;
  std::string enode_string = raw_enode.substr(start_pos, len_enode);

  BOOST_LOG_TRIVIAL(debug) << "EthereumManager: create_network, enode_string="
                           << raw_enode;

  network_config->set_enode(enode_string + "" + network_config->join_ip() +
                            ":" + std::to_string(network_config->peer_port()));

  config = std::move(network_config);

  return 0;
}

auto EthereumManager::join_network(
    const std::string &name, std::unique_ptr<BcManagerNetworkConfig> &config)
    -> int {
  unsigned short rpc_port = config_.rpc_port();
  unsigned short peer_port = config_.peer_port();
  std::string join_ip = config_.join_ip();

  while (port_in_use(rpc_port)) {
    rpc_port++;
  }
  while (port_in_use(peer_port)) {
    peer_port++;
  }

  EthereumNetworkConfig *network_config = (EthereumNetworkConfig *)config.get();
  network_config->init(rpc_port, peer_port, join_ip);

  // Start node
  std::string start_cmd = "bash " + config_.start_script_path() + " " +
                          kContainerName + "_" + name + " " +
                          std::to_string(network_config->rpc_port()) + " " +
                          std::to_string(network_config->peer_port()) + " " +
                          config_.docker_file_path();
  std::string join_cmd = "docker exec -it " + kContainerName + "_" + name +
                         R"( /bin/bash -c  "geth --exec \"admin.addPeer(')" +
                         network_config->enode() + R"(')\")" +
                         " attach eth-data/geth.ipc\"";
  BOOST_LOG_TRIVIAL(debug) << join_cmd;

  if (system(start_cmd.c_str()) != 0) {
    BOOST_LOG_TRIVIAL(debug)
        << "EthereumManager: join_network, start ethereum node failed!";
    return 1;
  }

  bool exec_successful;
  std::string response =
      retry_exec(join_cmd.c_str(), &exec_successful, TEN_SEC);
  if (!exec_successful) {  // time out
    std::string error_msg =
        "TIMEOUT during execution of the following command:\n";
    error_msg = error_msg + join_cmd + "\nReturn code was: " + response;
    BOOST_LOG_TRIVIAL(debug) << error_msg;
    return 1;
  }

  BOOST_LOG_TRIVIAL(debug) << "EthereumManager: join_network, enode_string="
                           << network_config->enode() << std::endl
                           << "Response: " << response;

  // Get enode from joining node
  std::string enode_cmd = "docker exec -it " + kContainerName + "_" + name +
                          " /bin/bash -c  \"geth --exec admin.nodeInfo.enode "
                          "attach eth-data/geth.ipc\"";

  std::string raw_enode =
      retry_exec(enode_cmd.c_str(), &exec_successful, TEN_SEC);
  if (!exec_successful) {  // time out
    std::string error_msg =
        "TIMEOUT during execution of the following command:\n";
    error_msg = error_msg + enode_cmd + "\nReturn code was: " + raw_enode;
    BOOST_LOG_TRIVIAL(debug) << error_msg;
    return 1;
  }

  //  std::string raw_enode = exec(enode_cmd.c_str());
  const size_t start_pos = 6;
  const size_t len_enode = raw_enode.find('@') - 5;
  std::string enode_string = raw_enode.substr(start_pos, len_enode);

  BOOST_LOG_TRIVIAL(debug) << "EthereumManager: join_network, enode_string="
                           << raw_enode;

  network_config->set_enode(enode_string + "" + network_config->join_ip() +
                            ":" + std::to_string(network_config->peer_port()));

  return 0;
}

auto EthereumManager::leave_network(
    const std::string &name, std::unique_ptr<BcManagerNetworkConfig> &config)
    -> int {
  (void)config;
  std::string shutdown_cmd1 = "docker stop " + kContainerName + "_" + name;
  std::string shutdown_cmd2 =
      "docker container rm " + kContainerName + "_" + name;

  if (system(shutdown_cmd1.c_str()) != 0) {
    BOOST_LOG_TRIVIAL(debug) << "EthereumManager: stop ethereum node failed!";
  }

  if (system(shutdown_cmd2.c_str()) != 0) {
    BOOST_LOG_TRIVIAL(debug) << "EthereumManager: remove ethereum node failed!";
  }

  return 0;
}
