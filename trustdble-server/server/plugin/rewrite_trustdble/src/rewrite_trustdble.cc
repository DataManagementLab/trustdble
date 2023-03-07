#include "rewrite_trustdble.h"
#include "trustdble_utils/encoding_helpers.h"
#include <cstdlib>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <sys/stat.h>

void rewrite_query(std::string &orig, REWRITE_QUERY_TUPLE rewrite_rule) {
  orig.replace(orig.begin(), orig.begin() + rewrite_rule.SHARED.length(),
               rewrite_rule.UNSHARED);
}

void generateConfig(const std::string &config_path, const std::string &bctype,
                    const std::string &dependency_dir) {

  static int offset_number = 0;

  std::ofstream file;
  file.open(config_path);

  if (bctype.compare("ETHEREUM") == 0) {
    int rpc_port = 8000 + offset_number;
    int peer_port = 30303 + offset_number;

    offset_number++;

    file << "[Adapter-Ethereum]" << std::endl;
    file << "connection-url=http://" << std::getenv("MYSQL_ROOT_HOST") << ":"
         << rpc_port << std::endl;
    file << "account=0x542BB0f7035f4bf7dB677a54B73e5d0514B9bfBC" << std::endl;
    file << "max-waiting-time=300" << std::endl;
    file << "script-path=" << dependency_dir
         << "/trustdble-adapters-src/ethereum/scripts" << std::endl;
    file << "contract-path=" << dependency_dir
         << "/trustdble-adapters-src/ethereum/contract/truffle/build/contracts/"
            "SimpleStorage.json"
         << std::endl;

    file << std::endl;

    file << "[Manager-Ethereum]" << std::endl;
    file << "rpc-port=" << rpc_port << std::endl;
    file << "peer-port=" << peer_port << std::endl;
    file << "join-ip=" << std::getenv("ETH_JOIN_IP") << std::endl;
    file << "start-script-path=" << dependency_dir
         << "/blockchain-manager-src/ethereum/scripts/start-network.sh"
         << std::endl;
    file << "docker-file-path=" << dependency_dir
         << "/blockchain-manager-src/ethereum/scripts/ethereum-docker"
         << std::endl;

  } else if (bctype.compare("STUB") == 0) {
    std::stringstream path;
    path << std::getenv("STUB_PATH") << "/stub_data/";
    mkdir(path.str().c_str(), 0777); // Should be done with bc manager in future
    path << config_path.substr(config_path.find_last_of('/') + 1,
                               config_path.length() -
                                   (config_path.find_last_of('/') + 1) -
                                   4); // append meta/data + databasename

    file << "[Adapter-Stub]" << std::endl;
    file << "data-path=" << path.str() << "/" << std::endl;
    file << "blocksize=10" << std::endl;

    mkdir(path.str().c_str(), 0777); // Should be done with bc manager in future
  }

  file.close();
}
