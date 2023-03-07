#include "manager_stub/manager_stub.h"

#include <boost/algorithm/string/predicate.hpp>
#include <boost/asio.hpp>
#include <boost/bind/bind.hpp>
#include <boost/filesystem.hpp>
#include <boost/log/trivial.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <boost/property_tree/ptree.hpp>
#include <fstream>
#include <iostream>

#define NUM_RETRY 10
#define TIME_OUT 100

using namespace boost::placeholders;

auto update_members(std::string &network_name, bool b, int *count) -> bool;

StubManager::StubManager() = default;
StubManager::~StubManager() = default;

auto StubManager::shutdown() -> bool { return true; }

auto StubManager::init(const std::string &config_path) -> bool {
  // Save config path into config
  config_.init(config_path);
  return true;
}

/**
 * @brief Create a new folder inside the stub-network with the given name.
 * Then we store the full path to this new folder in the network_config.
 * If the folder already exists or we cant create it we return the error code 1
 * otherwise 0.
 *
 * @return int
 */
auto StubManager::create_network(
    const std::string &name, std::unique_ptr<BcManagerNetworkConfig> &config)
    -> int {
  // Init method has already test the stub network folder exists
  // Create the network folder
  // The path of stub_network_path() should end with /
  // and network_name as a sub path should also end with /
  std::string network_name = config_.stub_network_path() + name + "/";

  // The folder to be created is alreday exists => failed
  if (boost::filesystem::is_directory(network_name)) {
    BOOST_LOG_TRIVIAL(debug)
        << "STUBManager: create network failed, network alreday exists";
    return 1;
  }

  // Or create network directory failed
  if (!boost::filesystem::create_directory(network_name)) {
    BOOST_LOG_TRIVIAL(debug)
        << "STUBManager: create network failed, cannot create network";
    return 1;
  }

  // Create the config.ini file and write members = 1;
  std::string filename = network_name + "config.ini";
  std::ofstream ofs;
  ofs.open(filename, std::ios::out);
  if (!ofs.is_open()) {
    // Remove the create network folder
    boost::filesystem::path p(network_name);
    boost::filesystem::remove_all(p);
    BOOST_LOG_TRIVIAL(debug) << "STUBManager: create network failed, cannot "
                                "open network config file";
    return 1;
  }
  ofs << "members= 1" << std::endl;
  ofs.close();

  // Store the network name to this new folder in the network_config
  std::unique_ptr<StubNetworkConfig> network_config =
      std::make_unique<StubNetworkConfig>();
  network_config->init(name);
  config = std::move(network_config);

  BOOST_LOG_TRIVIAL(debug) << "STUBManager: create network success, path= "
                           << network_name;
  BOOST_LOG_TRIVIAL(debug)
      << "STUBManager: create network success, config file = " << filename;

  return 0;
}

auto StubManager::join_network(const std::string &name,
                               std::unique_ptr<BcManagerNetworkConfig> &config)
    -> int {

  // Case the BcManagerNetworkConfig point to StubNetworkConfig point
  StubNetworkConfig *snc = (StubNetworkConfig *)config.get();
  std::string network_name = snc->network_name();
  network_name = config_.stub_network_path() + network_name + "/";

  // Update the config.ini file: members = members + 1;
  int count = 0; // Initial value doesn't matter, it will be set from read
                 // files
  // Update config file
  if (!update_members(network_name, true, &count)) {
    return 1;
  }

  BOOST_LOG_TRIVIAL(debug) << "STUBManager: joining network " << name
                           << " successful, current sum of members= " << count;
  return 0;
}

auto StubManager::leave_network(const std::string &name,
                                std::unique_ptr<BcManagerNetworkConfig> &config)
    -> int {

  // Case the BcManagerNetworkConfig point to StubNetworkConfig point
  StubNetworkConfig *snc = (StubNetworkConfig *)config.get();
  std::string network_name = snc->network_name();
  network_name = config_.stub_network_path() + network_name + "/";

  // Cpdate the config.ini file: members = members - 1;
  int count = 0;
  // Update config file
  if (!update_members(network_name, false, &count)) {
    return 1;
  }

  if (count < 1) {
    BOOST_LOG_TRIVIAL(debug)
        << "STUBManager: leaving network successful, no members";
    BOOST_LOG_TRIVIAL(debug) << "STUBManager: removing network successful";
  } else {
    BOOST_LOG_TRIVIAL(debug) << "STUBManager: leaving network " << name
                             << " successful, current sum of members=" << count;
  }

  return 0;
}

/**
 * @brief Update the count of members in the given file.
 * Increase or decrease under conditional control b
 *
 * @param file_path The path of the network, which include the config.ini file,
 * which to be update
 * @param b Conditional control. if b=true, increase the count of members
 * and if b=false decrease the count of members
 * @param count Save the count of members
 *
 * @return auto
 */
auto update_members(std::string &network_name, bool b, int *count) -> bool {
  *count = b ? 1 : -1;
  std::string state = b ? "joining" : "leaving";
  std::string filename = network_name + "config.ini";

  // Test if the network name folder exists
  // and test if the config.ini file exists.
  if (!boost::filesystem::is_directory(network_name) ||
      !boost::filesystem::exists(filename)) {
    BOOST_LOG_TRIVIAL(debug)
        << "STUBManager: " << state
        << " network failed, network/config file not exists";
    return false; // network not exists or config file not exists
  }

  // Create folder as mutex to lock the update of the config file
  std::string directory_mutex = network_name + "directory_mutex";
  int retry = 0;

  // Busy waiting if directory_mutex is exists (as mutex locked)
  // Allowed 10 retry
  while (boost::filesystem::is_directory(directory_mutex) &&
         retry++ < NUM_RETRY) {
    sleep(TIME_OUT);
  }

  // After 10 tries, cant get the lock
  if (boost::filesystem::is_directory(directory_mutex) ||
      !boost::filesystem::create_directory(directory_mutex)) {
    BOOST_LOG_TRIVIAL(debug) << "STUBManager: " << state
                             << " network failed, cannot read config file";
    return false;
  }

  // Create the directory_mutex success as lock the config file
  // Read file and get the members
  std::ifstream ifs;
  ifs.open(filename, std::ios::in);

  std::string str;
  int members = 0;
  if (!(ifs >> str >> members) || members < 1) {
    BOOST_LOG_TRIVIAL(debug) << "STUBManager: " << state
                             << " network failed, read config file failed";
    boost::filesystem::path p(directory_mutex);
    boost::filesystem::remove_all(p); // remove the directory_mutex as unlock
    return false;
  }
  ifs.close();
  *count += members; // increase/decrease the members

  // Update the file with new members
  std::ofstream ofs;
  ofs.open(filename, std::ios::out);
  ofs << "members= " << *count << std::endl;
  ofs.close();

  // if count(members) < 1, then delete the network (for leaving network)
  if (*count < 1) {
    boost::filesystem::path p(network_name);
    boost::filesystem::remove_all(p);
  } else {
    // Just remove the directory_mutex as unlock
    boost::filesystem::path p(directory_mutex);
    boost::filesystem::remove_all(p);
  }

  return true;
}