#include "manager_fabric/manager_fabric.h"

#include "manager_utils/shell_helpers.h"
#include "string"

///////// Healpers //////////////////////////////////////////

auto is_forbidden_char(char c) -> auto {
  return !((isdigit(c) != 0) || (islower(c) != 0) || c == '-' || c == '.');
}

auto to_channel_name(const std::string &name) -> std::string {
  std::string channel_name = name;
  std::transform(channel_name.begin(), channel_name.end(), channel_name.begin(),
                 [](unsigned char c) { return std::tolower(c); });
  size_t first_letter =
      channel_name.find_first_of("abcdefghijklmnopqrstuvwxyz");
  channel_name = channel_name.substr(first_letter);
  std::replace(channel_name.begin(), channel_name.end(), '_', '-');
  channel_name.erase(std::remove_if(++channel_name.begin(), channel_name.end(),
                                    &is_forbidden_char),
                     channel_name.end());
  boost::uuids::random_generator gen;
  boost::uuids::uuid id = gen();
  channel_name.append("-").append(boost::uuids::to_string(id));
  return channel_name;
}

///////// FabricManager Implementation //////////////////////

// Constructor
FabricManager::FabricManager() = default;

// Destructor
FabricManager::~FabricManager() = default;

auto FabricManager::init(const std::string &config_path) -> bool {
  // Init Ethereum config
  BOOST_LOG_TRIVIAL(debug) << "FabricManager: Init, config-path="
                           << config_path;
  config_.init(config_path);

  return true;
}

auto FabricManager::shutdown() -> bool {
  // don't remove network, but just stop the manager

  return true;
}

auto FabricManager::create_network(
    const std::string &name, std::unique_ptr<BcManagerNetworkConfig> &config)
    -> int {
  BOOST_LOG_TRIVIAL(debug) << "fabric: create network";
  std::string channel_name = to_channel_name(name);
  if (channel_name.length() > kMaxChannelNameLength) {
    BOOST_LOG_TRIVIAL(debug)
        << "FabricManager: create network failed!"
        << "the channel name " << channel_name << " has more than "
        << kMaxChannelNameLength << " characters";
    return 1;
  }
  // we don't need to check, if the test network already exists, because the
  // test-network will check, if the network is already running and will then
  // only add the channel
  std::string start_cmd = "bash " + config_.start_script_path() + " up " +
                          config_.test_network_dir() + " --network-name " +
                          channel_name;

  if (system(start_cmd.c_str()) != 0) {
    return 1;
  }

  std::unique_ptr<FabricNetworkConfig> network_config =
      std::make_unique<FabricNetworkConfig>();
  std::string crypto_path = config_.test_network_dir() +
                            "/organizations/peerOrganizations/org1.example.com";
  std::string cert_path =
      crypto_path + "/users/User1@org1.example.com/msp/signcerts/cert.pem";
  std::string key_path =
      crypto_path + "/users/User1@org1.example.com/msp/keystore/";
  std::string tls_cert_path = crypto_path + "/peers/" + name + "/tls/ca.crt";
  network_config->init(channel_name, "Org1MSP", cert_path, key_path,
                       tls_cert_path, name, config_.test_network_dir());

  config = std::move(network_config);

  if (join_network(name, config) != 0) {
    return 1;
  }

  return 0;
}

auto FabricManager::join_network(
    const std::string &name, std::unique_ptr<BcManagerNetworkConfig> &config)
    -> int {
  // check bc type
  FabricNetworkConfig *network_config = (FabricNetworkConfig *)config.get();

  unsigned short peer_port = config_.starting_peer_port();
  unsigned short operations_port = config_.starting_operations_port();

  while (port_in_use(peer_port)) {
    peer_port++;
  }
  while (port_in_use(operations_port)) {
    operations_port++;
  }

  network_config->set_peer_port(peer_port);
  network_config->set_operations_port(operations_port);
  network_config->set_peer_endpoint("localhost:" + std::to_string(peer_port));

  BOOST_LOG_TRIVIAL(debug) << "fabric: join network";
  std::string start_cmd =
      "bash " + config_.start_script_path() + " join " +
      config_.test_network_dir() + " --network-name " +
      network_config->channel_name() + " --peer-name " + name +
      " --peer-port " + std::to_string(network_config->peer_port()) +
      " --operations-port " + std::to_string(network_config->operations_port());

  if (system(start_cmd.c_str()) != 0) {
    BOOST_LOG_TRIVIAL(debug) << "error joining peer";
    return 1;
  }

  return 0;
}

auto FabricManager::leave_network(
    const std::string &name, std::unique_ptr<BcManagerNetworkConfig> &config)
    -> int {
  FabricNetworkConfig *network_config = (FabricNetworkConfig *)config.get();

  // leave with the peer of the given name, but leave other stuff intact
  std::string remove_peer_cmd = "bash " + config_.start_script_path() +
                                " remove " + " --peer-name " + name;

  if (system(remove_peer_cmd.c_str()) != 0) {
    BOOST_LOG_TRIVIAL(debug) << "FabricManager: remove fabric peer failed!";
    return 1;
  }

  // this command fails, if docker ps finds no remaining containers on the
  // same channel
  std::string peers_channel_cmd =
      "docker ps -q --filter "
      "\"label=" +
      kChannelContainerLabel + "=" + network_config->channel_name() +
      "\""
      "| grep -q .";

  if (system(peers_channel_cmd.c_str()) != 0) {
    std::string remove_blockfile_cmd =
        "rm " + config_.test_network_dir() + "/channel-artifacts/" +
        network_config->channel_name() + ".block";

    if (system(remove_blockfile_cmd.c_str()) != 0) {
      BOOST_LOG_TRIVIAL(debug)
          << "FabricManager: remove channel blockfile failed!";
      return 1;
    }
  }

  // this command fails, if docker ps finds no containers joined to
  // the test-network with the method join_network
  std::string peers_test_network_cmd =
      "docker ps -q --filter "
      "\"label=com.trustdble.fabric-network=test-network\""
      "| grep -q .";

  if (system(peers_test_network_cmd.c_str()) != 0) {
    std::string remove_network_cmd = "bash " + config_.start_script_path() +
                                     " down " + config_.test_network_dir();
    if (system(remove_network_cmd.c_str()) != 0) {
      BOOST_LOG_TRIVIAL(debug) << "FabricManager: remove network failed!";
      return 1;
    }
  }

  return 0;
}