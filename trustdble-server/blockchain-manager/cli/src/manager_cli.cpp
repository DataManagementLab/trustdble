#include <iostream>
#include <string>

#include "bc_manager_factory/bc_manager_factory.h"
#include "bc_manager_interface/bc_manager_interface.h"

using namespace std;
using namespace blockchain_manager;

// NOLINTNEXTLINE(bugprone-exception-escape)
auto main(int argc, char *argv[]) -> int {
  std::unique_ptr<BcManager> manager = nullptr;
  std::unique_ptr<BcManagerNetworkConfig> network_config = nullptr;

  std::string network_name = "test_net";

  std::string bctype;
  std::string param_input;
  std::string cmd;

  cout << endl
       << "### Simple CLI for BlockchainManager ###" << endl
       << endl
       << "Enter a Blockchain Type: ";

  cin >> bctype;

  std::string config_path;
  if (argc > 1) {
    config_path = argv[1];
  } else {
    config_path = "./config.ini";
  }

  // Create manager instance of specified type
  manager =
      BcManagerFactory::create_manager(BcManagerFactory::getBC_TYPE(bctype));
  if (!manager) {
    cout << endl
         << "Unkown blockchain type: " << bctype << endl
         << "Supported types are: " << endl;
    for (const auto &k_bc_type_string : kBcTypeStrings) {
      cout << k_bc_type_string.first << endl;
    }
    return 1;
  }
  // Initialize manager with configuration
  manager->init(config_path);

  cout << endl
       << "Usage:" << endl
       << "\tcreate \t\tCreates a new network and outputs the network config "
          "as JSON"
       << endl
       << "\tjoin \t\tJoins an existing network specified via network config "
          "as JSON"
       << endl
       << "\tleave \t\tLeaves the network specified via network name" << endl
       << endl;

  cout << "Command: ";
  cin >> cmd;

  // Creating a new network
  if (std::strcmp(cmd.c_str(), "create") == 0) {
    cout << "Enter network name: ";
    cin >> network_name;

    cout << "Creating network " << network_name << " ... " << endl;

    if (manager->create_network(network_name, network_config) != 0) {
      cout << "Creating network failed!" << endl;
      return 1;
    }

    cout << endl
         << "Network created!" << endl
         << "Configuration: " << endl
         << network_config->get_json() << endl;
  }

  // Joining a existing network
  else if (std::strcmp(cmd.c_str(), "join") == 0) {
    cout << "If you want to join a local network, you might need to use a "
            "different name. E.g.: <network_name>_node2 "
         << endl
         << "Enter network name: ";
    cin >> network_name;

    cout << "Enter network config as json: ";
    getline(cin,
            param_input);  // Need to be done twice to read a space that is left
                           // from normal cin input that was executed before.
    getline(cin, param_input);
    cout << "Joining network " << param_input << " ... " << endl;

    network_config = std::make_unique<BcManagerNetworkConfig>();
    network_config->read(param_input);
    if (manager->join_network(network_name, network_config) != 0) {
      cout << "Joining network failed!" << endl;
      return 1;
    }

    cout << endl
         << "Joined network!" << endl
         << "Configuration: " << endl
         << network_config->get_json() << endl;
  }

  // Leaving a existing network
  else if (std::strcmp(cmd.c_str(), "leave") == 0) {
    cout << "Enter network name: ";
    cin >> network_name;
    cout << "Leaving network " << network_name << " ... " << endl;

    if (bctype == "STUB" || bctype == "FABRIC") {
      cout << "Enter network config as json: ";
      getline(cin, param_input);
      getline(cin, param_input);
      cout << "Leaving network " << param_input << " ... " << endl;
      network_config = std::make_unique<BcManagerNetworkConfig>();
      network_config->read(param_input);
    }

    if (manager->leave_network(network_name, network_config) != 0) {
      cout << "Leaving network failed!" << endl;
      return 1;
    }

    cout << "Left the network!" << endl;
  }

  else {
    cout << "Wrong command!" << endl;
  }

  manager->shutdown();
  manager.reset();

  return 0;
}