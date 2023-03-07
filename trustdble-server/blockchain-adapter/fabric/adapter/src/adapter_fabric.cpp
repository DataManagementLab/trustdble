/*! \addtogroup group31
 *  @{
 */
#include "adapter_fabric/adapter_fabric.h"

#include "adapter_utils/encoding_helpers.h"

/* 
 * ---- HELPER METHODS ----------------------------------
 */
/**
 * @brief Helper-Method to convert an bytes into its hex-representation.
 *
 * @param bytes The bytes to be converted.
 *
 * @return Hex-encoded string
 */
auto bytes_to_hex(const BYTES &bytes) -> std::string {
  std::stringstream ss;
  ss << std::hex;
  unsigned i = 0;
  for (; i < bytes.size; i++) {
    ss << std::setw(2) << std::setfill('0') << (int)(bytes.value[i]);
  }
  return ss.str();
}

/*
 * ---- Fabric IMPLEMENTATION ----------------------------------
 */

FabricAdapter::FabricAdapter() = default;
FabricAdapter::~FabricAdapter() {
  if (client_.isInit()) {
    client_.close();
  }
}

auto FabricAdapter::init(const std::string &config_path) -> bool {
  BOOST_LOG_TRIVIAL(debug) << "fabric: Init, config-path=" << config_path;
  config_.init(config_path);
  return true;
}

auto FabricAdapter::init(const std::string &config_path,
                         const std::string &network_config) -> bool {
  BOOST_LOG_TRIVIAL(debug) << "fabric: Init, config-path=" << config_path;
  config_.init(config_path);
  config_.set_network_config(network_config);
  return true;
}

auto FabricAdapter::shutdown() -> bool {
  if (client_.isInit()) {
    return client_.close() == 0;
  }
  return true;
}

auto FabricAdapter::put(std::map<const BYTES, const BYTES> &batch) -> int {
  if (client_.isInit()) {
    auto error = client_.put(batch);

    if (error == 0) {
      BOOST_LOG_TRIVIAL(debug) << "fabric: Put, Success";
      // The interface expects successfully put key value pairs to be removed
      // from the map
      batch.clear();
      return 0;
    }
    BOOST_LOG_TRIVIAL(debug) << "fabric: Put failed!";

    return 1;
  }
  BOOST_LOG_TRIVIAL(debug) << "fabric: FabricClient is not initialized!";

  return 1;
}

auto FabricAdapter::get(const BYTES &key, BYTES &result) -> int {
  if (client_.isInit()) {
    auto error = client_.get(key, result);

    if (error != 0) {
      BOOST_LOG_TRIVIAL(debug) << "fabric: No value for key "
                               << bytes_to_hex(key).c_str() << " found";
      return 1;
    }

    BOOST_LOG_TRIVIAL(debug) << "fabric: GET, Success";
    return 0;
  }
  BOOST_LOG_TRIVIAL(debug) << "fabric: FabricClient is not initialized!";

  return 1;
}

auto FabricAdapter::remove(const BYTES &key) -> int {
  std::list<BYTES> batch({key});
  return remove_batch(batch);
}

auto FabricAdapter::remove_batch(std::list<BYTES> &batch) -> int {
  if (client_.isInit()) {
    auto error = client_.remove(batch);

    if (error == 0) {
      BOOST_LOG_TRIVIAL(debug) << "fabric: Remove, Success";
      return 0;
    }
    BOOST_LOG_TRIVIAL(debug)

        << "fabric: Remove failed! Status message: ";  //<<
                                                       // return_value.c_str();

    return 1;
  }
  BOOST_LOG_TRIVIAL(debug) << "fabric: FabricClient is not initialized!";

  return 1;
}

auto FabricAdapter::get_all(std::map<const BYTES, BYTES> &results) -> int {
  if (client_.isInit()) {
    auto error = client_.getAll(results);

    if (error != 0) {
      BOOST_LOG_TRIVIAL(debug) << "fabric: Get ALL failed";
      return 1;
    }

    BOOST_LOG_TRIVIAL(debug) << "fabric: GET ALL, Success";
    return 0;
  }
  BOOST_LOG_TRIVIAL(debug) << "fabric: FabricClient is not initialized!";

  return 1;
}

auto FabricAdapter::create_table(const std::string &name,
                                 std::string &tableAddress) -> int {
  // deploy contract for table return contract name
  tableAddress = string_to_hex(name);

  load_table(name, "");

  return 0;
}

auto FabricAdapter::load_table(const std::string &name,
                               const std::string &tableAddress) -> int {
  (void)tableAddress;

  BOOST_LOG_TRIVIAL(debug) << "fabric: DeployContract";

  std::string contract_name = config_.channel_name();

  std::string deploy_command =
      config_.adapters_path() + "/fabric/scripts/deployContract.sh " +
      config_.channel_name() + " " + contract_name + " " +
      config_.peer_endpoint() + " " + config_.test_network_path() + " " +
      config_.adapters_path();

  if (system(deploy_command.c_str()) != 0) {
    return 1;
  }

  std::string hex_table_name = string_to_hex(name);

  // Init fabric client for communicationget_all with blockchain network
  return client_.init(config_.channel_name(), contract_name, config_.msp_id(),
                      config_.cert_path(), config_.key_path(),
                      config_.tls_cert_path(), config_.peer_endpoint(),
                      config_.gateway_peer(), hex_table_name);
}

auto FabricAdapter::drop_table() -> int {
  std::map<const BYTES, BYTES> results;
  if (this->get_all(results) == 0) {
    std::list<BYTES> batch;
    for (auto iter : results) {
      batch.push_back(std::get<0>(iter));
    }
    remove_batch(batch);
  }
  BOOST_LOG_TRIVIAL(debug) << "fabric: DropTable not implemented correctly. "
                              "Deletes only all entries!";
  return 0;
}
/** @} */
