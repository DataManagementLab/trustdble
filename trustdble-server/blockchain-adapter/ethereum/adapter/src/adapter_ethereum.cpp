/*! \addtogroup group10
 *  @{
 */
#include "adapter_ethereum/adapter_ethereum.h"

#include "adapter_utils/encoding_helpers.h"
#include "adapter_utils/shell_helpers.h"

/*
 * ---- Ethereum IMPLEMENTATION ----------------------------------
 */

#define START_KEY_ID 0
#define BATCH_SIZE 100

//! The hash of the put method signature of trustdble ethereum contract
constexpr static auto kEthereumMethodHashPut = "0xdb82ecc3";
//! The hash of the get method signature of trustdble ethereum contract
constexpr static auto kEthereumMethodHashGet = "0x8eaa6ac0";
//! The hash of the get_all method signature of trustdble ethereum contract
constexpr static auto kEthereumMethodHashGetall = "0xb3055e26";
//! The hash of the remove method signature of trustdble ethereum contract
constexpr static auto kEthereumMethodHashRemove = "0x95bc2673";
//! The hash of the putBatch method signature of trustdble ethereum contract
constexpr static auto kEthereumMethodHashPutBatch = "0x410f08ab";
//! The hash of the getBatch method signature of trustdble ethereum contract
constexpr static auto kEthereumMethodHashGetBatch = "0xfe918e68";
//! The default gas value of 7000000 for transaction in hex
constexpr static auto kEthereumGas = "0x6ACFC0";

std::mutex EthereumAdapter::m;

// Constructur
EthereumAdapter::EthereumAdapter() = default;

// Destructur
EthereumAdapter::~EthereumAdapter() = default;

auto EthereumAdapter::init(const std::string &config_path) -> bool {
  // init Ethereum config
  BOOST_LOG_TRIVIAL(debug) << "Ethereum Adapter: Init, config-path="
                           << config_path;
  // verify configuration path
  if (!verify_config_path(config_path)) {
    BOOST_LOG_TRIVIAL(debug)
        << "EthereumAdapter: init | "
           "initialization stopped, fail to verify configuration path";
    return false;
  }

  config_.init_path(config_path);
  return init();
}

auto EthereumAdapter::init(const std::string &config_path,
                           const std::string &network_config) -> bool {
  // verify configuration path
  if (!verify_config_path(config_path)) {
    BOOST_LOG_TRIVIAL(debug)
        << "EthereumAdapter: init | "
           "initialization stopped, fail to verify configuration path";
    return false;
  }
  // init adapter with config_path
  config_.init_path(config_path);

  // verify network configuration
  if (!verify_network_config(network_config)) {
    BOOST_LOG_TRIVIAL(debug)
        << "EthereumAdapter: init | "
           "initialization stopped, fail to verify network configuration";
    return false;
  }
  // set network_config with connection-url (join-ip + rpc-port)
  config_.set_network_config(network_config);

  return init();
}

auto EthereumAdapter::shutdown() -> bool {
  curl_easy_cleanup(curl_);
  return true;
}

auto EthereumAdapter::put_batch(std::map<const BYTES, const BYTES> &batch) -> int {
  // Init of variables
  std::map<const BYTES, const BYTES>::iterator it;
  int batch_id = 0;
  std::string key_string;
  std::string value_offset;
  int value_offset_counter = 0;
  std::string value_string;

  // iterate over all pairs in the batch
  for (it = batch.begin(); it != batch.end(); it++) {

    // construct the string that holds all keys
    std::string padded_key = convert_to_32byte(byte_array_to_hex(it->first.value, it->first.size));
    key_string.append(padded_key);

    // construct value offset string
    std::string curr_offset = convert_to_32byte(int_to_hex(batch.size()*32+64*value_offset_counter));
    value_offset.append(curr_offset);
    value_offset_counter++;

    // construct value string consisting of value length and the respective value
    std::string val_len = int_to_hex(it->second.size);
    value_string.append(convert_to_32byte(val_len));
    value_string.append(convert_to_32byte(byte_array_to_hex(it->second.value, it->second.size)));
  }

  update_nonce();

  BOOST_LOG_TRIVIAL(debug) << "Ethereum Adapter: Put, Nonce is " + std::to_string(nonce_.load());
  RpcParams params;
  params.method = "eth_sendTransaction";
  params.transaction_ID = std::to_string(batch_id++);

  std::string data = int_to_hex(VALUE_SIZE) + int_to_hex(batch.size()*32+96) + int_to_hex(batch.size()) + key_string +
                     int_to_hex(batch.size()) + value_offset + value_string;

  params.data = kEthereumMethodHashPutBatch + data;

  // Make call to the blockchain to write all elements from the batch as one transaction
  call(params, true);

  return 0;
}

auto EthereumAdapter::put(std::map<const BYTES, const BYTES> &batch) -> int {
  // Init of variables
  // std::map<const BYTES, const BYTES>::iterator it;
  std::map<RpcParams, bool> batch_transform;
  std::map<RpcParams, std::string> key_map;
  int batch_elem_id = 0;

  // iterate over all pairs in the map
  for (auto &it : batch) {
    // update nonce
    update_nonce();
    BOOST_LOG_TRIVIAL(debug)
        << "Ethereum Adapter: Put, Nonce is " + std::to_string(nonce_.load());

    std::string padded_key =
        convert_to_32byte(byte_array_to_hex(it.first.value, it.first.size));
    std::string offset = int_to_hex(VALUE_SIZE);
    std::string val_len = int_to_hex(it.second.size);

    RpcParams params;
    params.method = "eth_sendTransaction";
    params.data =
        std::string(kEthereumMethodHashPut)
            .append(padded_key)
            .append(offset)
            .append(val_len)
            .append(byte_array_to_hex(it.second.value, it.second.size));

    // Since the transaction ID is not yet set, set it to the value of the
    // batchElem_ID on an interim basis so that we have a unique key for the
    // maps
    params.transaction_ID = std::to_string(batch_elem_id++);

    // fill the two maps that are the input for the first call function
    batch_transform.emplace(params, true);
    key_map.emplace(params, byte_array_to_hex(it.first.value, it.first.size));
  }

  const auto rpc_batch = createRpcBatch(batch_transform, key_map);

  const std::vector<std::string> response =
      sendRpcBatch(rpc_batch.first, rpc_batch.second);

  // response contains the keys where the insertion failed
  // using this information, successfully inserted key-value pairs are removed
  // from the batch
  if (!response.empty()) {
    // std::map<const BYTES, const BYTES>::iterator it;

    for (auto it = batch.begin(); it != batch.end(); it++) {
      if (std::find(response.begin(), response.end(),
                    byte_array_to_hex(it->first.value, it->first.size)) ==
          response.end()) {
        batch.erase(it->first);
      }
    }
    return 1;
  }
  return 0;
}

auto EthereumAdapter::get(const BYTES &key, BYTES &result) -> int {
  std::string padded_key =
      convert_to_32byte(byte_array_to_hex(key.value, key.size));

  RpcParams params;
  params.method = "eth_call";
  params.data = kEthereumMethodHashGet + padded_key;
  params.quantity_tag = "latest";

  const std::string response = call(params, false);
  // BOOST_LOG_TRIVIAL(debug) << "Ethereum Adapter: Get, Response: " <<
  // response;

  if (response.find("error") == std::string::npos) {
    BOOST_LOG_TRIVIAL(debug) << "Ethereum Adapter: Get, Successful!";

    // extract results
    std::regex rgx(".*\"result\":\"0x(\\w+)\".*");
    std::smatch match;

    if (std::regex_search(response.begin(), response.end(), match, rgx)) {
      int val_size =
          hex_to_int(((std::string)match[1]).substr(VALUE_SIZE, VALUE_SIZE)) *
          2;
      result.size =
          (((std::string)match[1]).substr(2 * VALUE_SIZE, val_size)).length() /
          2;
      unsigned char *new_result = new unsigned char[result.size];
      hex_to_byte_array(
          (((std::string)match[1]).substr(2 * VALUE_SIZE, val_size)),
          new_result);
      result = BYTES(new_result, result.size);
      delete[] new_result;
      return 0;
    }
    std::string key_str = std::string((const char *)key.value, key.size);
    BOOST_LOG_TRIVIAL(debug)
        << "Ethereum Adapter: Get, No value found for key: " << key_str;
    return 1;
  }
  BOOST_LOG_TRIVIAL(debug) << "Ethereum Adapter: Get, Failed: " << response;
  return 1;
}

auto EthereumAdapter::remove(const BYTES &key) -> int {
  // update nonce
  update_nonce();
  BOOST_LOG_TRIVIAL(debug) << "Ethereum Adapter: Remove, Nonce is " +
                                  std::to_string(nonce_.load());

  std::string padded_key =
      convert_to_32byte(byte_array_to_hex(key.value, key.size));
  RpcParams params;
  params.method = "eth_sendTransaction";

  params.data = kEthereumMethodHashRemove + padded_key;

  const std::string response = call(params, true);
  // BOOST_LOG_TRIVIAL(debug) << "Ethereum Adapter: Remove, response: " <<
  // response;

  if (response.find("error") == std::string::npos) {
    // check transaction receipt
    params.method = "eth_getTransactionReceipt";

    BOOST_LOG_TRIVIAL(debug) << "Ethereum Adapter: Remove, Successful!";
    return 0;
  }
  BOOST_LOG_TRIVIAL(debug) << "Ethereum Adapter: Remove, Failed: " << response;
  return 1;
}

auto EthereumAdapter::get_all(std::map<const BYTES, BYTES> &results) -> int {
  int key_id = START_KEY_ID;
  int batch_size = BATCH_SIZE;
  std::string hex_batch_size = int_to_hex(batch_size);

  bool read_end=false;
  while(!read_end){
    std::string hex_key_id = int_to_hex(key_id);
    key_id=key_id+batch_size;

    RpcParams params;
    params.method = "eth_call";
    params.data = kEthereumMethodHashGetBatch+hex_key_id+hex_batch_size;
    params.quantity_tag = "latest";

    BOOST_LOG_TRIVIAL(debug) << "Ethereum Adapter: Get_All, Response: "
                              << params.data;

    const std::string response = call(params, false);
    BOOST_LOG_TRIVIAL(debug) << "Ethereum Adapter: Get_All, Response: "
                              << response;

    std::string rpc_result;
    try {
      auto json = nlohmann::json::parse(response);
      rpc_result = json["result"];
      rpc_result = rpc_result.substr(2);  // remove 0x from front
      auto tmp = split(rpc_result);
      results.insert(tmp.begin(),tmp.end());
    } catch (std::exception &e) {
      BOOST_LOG_TRIVIAL(debug) << "Ethereum Adapter: Get_All, Failed: Can not "
                                  "parse TableScan response! Error: " << e.what();
      read_end=true;
    }
  }

  if (results.empty()) {
    return 1;
  }

  return 0;
}

auto EthereumAdapter::create_table(const std::string &name,
                                   std::string &tableAddress) -> int {
  if (name == tableName_) {
    return 1;
  }

  // Deploy Contract
  std::string cmd = "node " + config_.script_path() +
                    "/deploy_KV_contract.js " + accountAddress_ + " " +
                    config_.contract_path() + " " + config_.connection_url();
  tableAddress = exec(cmd.c_str());
  storedContractAddress_ = tableAddress;
  tableName_ = name;

  BOOST_LOG_TRIVIAL(debug)
      << "Ethereum Adapter: Create_Table, Contract Address: "
      << storedContractAddress_ << " for table: " << tableName_;

  // init nonce
  if (nonce_ == 0) {
    update_nonce();
  }

  BOOST_LOG_TRIVIAL(debug) << "Ethereum Adapter: Create_Table, Nonce is " +
                                  std::to_string(nonce_.load());

  return 0;
}

auto EthereumAdapter::load_table(const std::string &name,
                                 const std::string &tableAddress) -> int {
  if (!tableAddress.empty()) {
    storedContractAddress_ = tableAddress;
  } else {
    BOOST_LOG_TRIVIAL(debug)
        << "Ethereum Adapter: Load_Table, tableAddress is empty!";
    return 1;
  }

  tableName_ = name;

  BOOST_LOG_TRIVIAL(debug) << "Ethereum Adapter: Load_Table, Contract Address: "
                           << storedContractAddress_
                           << " for table: " << tableName_;

  // init nonce
  if (nonce_ == 0) {
    update_nonce();
  }

  BOOST_LOG_TRIVIAL(debug) << "Ethereum Adapter: Load_Table, Nonce is " +
                                  std::to_string(nonce_.load());

  return 0;
}

auto EthereumAdapter::drop_table() -> int {
  std::map<const BYTES, BYTES> results;
  if (this->get_all(results) == 0) {
    for (auto &result : results) {
      this->remove(result.first);
    }
  }
  BOOST_LOG_TRIVIAL(debug)
      << "Ethereum Adapter: Drop_Table, Not implemented correctly. "
         "Deletes only all entries!";
  return 0;
}

/*
 * ---- HELPER METHODS ----------------------------------
 */

auto EthereumAdapter::verify_config_path(const std::string &config_path)
    -> bool {
  // check is not empty
  if (config_path.empty()) {
    BOOST_LOG_TRIVIAL(debug) << "EthereumAdapter: verify_config_path | "
                                "config_path is an empty string";
    return false;
  }

  return true;
}

auto EthereumAdapter::verify_network_config(const std::string &network_config)
    -> bool {
  // check is not empty
  if (network_config.empty()) {
    BOOST_LOG_TRIVIAL(debug) << "EthereumAdapter: verify_network_config | "
                                "network_config is an empty string";
    return false;
  }

  // check structure = json
  if (!nlohmann::json::accept(network_config)) {
    BOOST_LOG_TRIVIAL(debug) << "EthereumAdapter: verify_network_config | "
                                "network_config is not a json string";
    return false;
  }

  // read config
  std::stringstream ss;
  ss << network_config;
  pt::ptree net_config;
  pt::read_json(ss, net_config);

  // check if join-ip in network config
  std::string join_ip = net_config.get<std::string>("Network.join-ip");
  if (join_ip.empty()) {
    BOOST_LOG_TRIVIAL(debug) << "EthereumAdapter: verify_network_config | "
                                "can't find Network.join-ip";
    return false;
  }

  return true;
}

auto EthereumAdapter::update_nonce() -> bool {
  RpcParams params;
  params.method = "eth_getTransactionCount";
  params.quantity_tag = "latest";
  std::string param = "\"" + accountAddress_ + R"(", "latest")";
  std::string method = "eth_getTransactionCount";

  auto response = call(param, method);
  try {
    auto json = nlohmann::json::parse(response);
    auto hex_count = json["result"].get<std::string>().substr(2);  // remove 0x
    auto tmp_nonce = strtoul(hex_count.c_str(), nullptr, ENCODED_BYTE_SIZE);
    tmp_nonce = std::max((unsigned long)0, tmp_nonce - 1);  // nonce starts at 0
    nonce_.store(tmp_nonce);
  } catch (std::exception &) {
    BOOST_LOG_TRIVIAL(debug) << "Ethereum Adapter: Update Nonce, Failed: Can "
                                "not parse eth_getTransactionCount response!";
  }
  return true;
}

auto EthereumAdapter::init() -> bool {
  /// \cond
  this->max_waiting_time_ =
      config_.max_waiting_time() * WAITING_TIME_IN_SEC;  // convert to ms
  m.lock();
  curl_ = curl_easy_init();
  curl_easy_setopt(curl_, CURLOPT_TCP_KEEPALIVE, 1L);
  curl_easy_setopt(curl_, CURLOPT_URL, config_.connection_url().c_str());
  curl_easy_setopt(curl_, CURLOPT_WRITEFUNCTION, write_callback);
  m.unlock();
  RpcParams params;
  params.method = "eth_accounts";

  std::string json;

  const std::string response = call(json, params.method);

  if (response.find("error") == std::string::npos) {
    BOOST_LOG_TRIVIAL(debug)
        << "Ethereum Adapter: Init, ListAccounts successful: " << response;

    // extract result
    std::regex rgx(".*\"result\":\\[\"(\\w+)\".*");
    std::smatch match;

    if (std::regex_search(response.begin(), response.end(), match, rgx)) {
      accountAddress_ = (std::string)match[1];
      BOOST_LOG_TRIVIAL(debug)
          << "Ethereum Adapter: Init, Set account to " << accountAddress_;
    } else {
      BOOST_LOG_TRIVIAL(debug)
          << "Ethereum Adapter: Init, No result for accounts";
      return false;
    }

    nonce_ = 0;

    return true;
  }

  BOOST_LOG_TRIVIAL(debug) << "Ethereum Adapter: Init, Failed: " << response;
  return false;
  /// \endcond
}

auto EthereumAdapter::convert_to_32byte(const std::string &data)
    -> std::string {
  std::stringstream ss;
  ss << std::hex << data << std::setfill('0')
     << std::setw(VALUE_SIZE - data.length()) << '0';
  return ss.str().substr(0, VALUE_SIZE);
}

auto EthereumAdapter::split(const std::string &response, int split_length)
    -> std::map<const BYTES, BYTES> {
  std::map<const BYTES, BYTES> ret;
  std::vector<std::string> keys;
  std::vector<std::string> values;

  if (!response.empty()) {
    unsigned long num_keys_values = hex_to_int(response.substr(
        split_length * 2,
        split_length));  // *2 to skip first 2 rows of meta data in response

    keys.reserve(num_keys_values);
    values.reserve(num_keys_values);

    // get keys
    for (unsigned long i = 0; i < num_keys_values; i++) {
      keys.push_back(response.substr(
          ((i + 3) * split_length),
          split_length));  // +3 to skip rows including meta-data
    }

    // get values
    std::string parsed_values =
        response.substr((num_keys_values + 4) *
                        split_length);  // +4 to skip rows including meta-data

    // split values by token
    std::string result = parsed_values;
    std::string token = "23232323";  // '####' in hex
    std::string value;

    while (static_cast<unsigned int>(!result.empty()) != 0U) {
      auto index = result.find(token);
      if (index != std::string::npos) {
        // get value
        values.push_back(result.substr(0, index));
        result = result.substr(index + token.size());
      } else {
        break;
      }
    }

    // fill result vector with keys and values
    for (unsigned long i = 0; i < num_keys_values; i++) {
      unsigned char *key = new unsigned char[keys.at(i).length() / 2];
      unsigned char *value = new unsigned char[values.at(i).length() / 2];
      hex_to_byte_array(keys.at(i), key);
      hex_to_byte_array(values.at(i), value);
      BYTES key_byte(key, keys.at(i).length() / 2);
      BYTES value_byte(value, values.at(i).length() / 2);

      ret.emplace(key_byte, value_byte);
      delete[] key;
      delete[] value;
    }
  }
  return ret;
}

auto EthereumAdapter::write_callback(char *contents, size_t size, size_t nmemb,
                                     void *userp) -> size_t {
  ((std::string *)userp)->append(contents, size * nmemb);
  return size * nmemb;
}

auto EthereumAdapter::parse_params_to_json(const RpcParams &params)
    -> std::string {
  std::vector<std::string> els;
  std::string json = "{";

  /// \cond
  if (!params.from.empty()) {
    els.push_back(R"("from":")" + params.from + "\"");
  }
  if (!params.data.empty()) {
    els.push_back(R"("data":")" + params.data + "\"");
  }
  if (!params.to.empty()) {
    els.push_back(R"("to":")" + params.to + "\"");
  }
  if (!params.gas.empty()) {
    els.push_back(R"("gas":")" + params.gas + "\"");
  }
  if (!params.gas_price.empty()) {
    els.push_back(R"("gasPrice":")" + params.gas_price + "\"");
  }
  if (params.nonce > 0) {
    std::stringstream ss;
    ss << "0x";
    ss << int_to_hex(params.nonce, 0);  // set to 0 to have no leading zeros
    els.push_back(R"("nonce":")" + ss.str() + "\"");
  }

  for (std::vector<int>::size_type i = 0; i < els.size(); i++) {
    json += els[i];
    if (i < els.size() - 1) {
      json += ",";
    }
  }

  return json + "}";
  /// \endcond
}

auto EthereumAdapter::call(RpcParams params, bool set_gas) -> std::string {
  std::string from_address = accountAddress_;

  params.from = from_address;
  if (params.to.empty()) {
    params.to = storedContractAddress_;
  }
  if (set_gas) {
    params.gas = kEthereumGas;
  }

  // Increment nonce to indicate that Ethereum should not replace a currently
  // pending transaction, but add as new transaction
  if (params.method == "eth_sendTransaction") {
    params.nonce = ++nonce_;
  }

  std::string json = parse_params_to_json(params);
  const std::string quantity_tag =
      params.quantity_tag.empty() ? "" : ",\"" + params.quantity_tag + "\"";
  json = json + quantity_tag;

  std::string response;

  return call(json, params.method);
}

auto EthereumAdapter::call(std::string &params, std::string &method)
    -> std::string {
  std::string read_buffer_call;
  const std::string post_data = R"({"jsonrpc":"2.0","id":1,"method":")" +
                                method + R"(","params":[)" + params + "]}";
  std::string read_buffer;

  if (curl_ != nullptr) {
    struct curl_slist *headers = nullptr;

    m.lock();
    headers = curl_slist_append(headers, "Content-Type: application/json");

    curl_easy_setopt(curl_, CURLOPT_HTTPHEADER, headers);
    curl_easy_setopt(curl_, CURLOPT_WRITEDATA, &read_buffer_call);
    curl_easy_setopt(curl_, CURLOPT_POSTFIELDS, post_data.c_str());
    CURLcode res = curl_easy_perform(curl_);
    m.unlock();
    if (res != CURLE_OK) {
      auto msg = "CURL perform() returned an error: " +
                 std::string(curl_easy_strerror(res));
      BOOST_LOG_TRIVIAL(debug) << "Ethereum Adapter: Call, " << msg;
    }

    if (method == "eth_sendTransaction") {
      nlohmann::json json_response;
      parseTX_response(read_buffer_call, json_response);
      auto transaction_id = json_response["result"].get<std::string>();
      BOOST_LOG_TRIVIAL(debug)
          << "Ethereum Adapter: Call, Transaction-ID: " << transaction_id;
      read_buffer = check_mining_result(transaction_id);
      if (!check_transaction_receipt(transaction_id)) {
        read_buffer = "error";
      }
    } else {
      read_buffer = read_buffer_call;
    }
  }
  return read_buffer;
}

void EthereumAdapter::parseTX_response(const std::string &read_buffer_call,
                                       nlohmann::json &json_response) {
  try {
    json_response = nlohmann::json::parse(read_buffer_call);

    if (json_response.contains("error")) {
      auto error_msg =
          json_response.at("error").at("message").get<std::string>();

      std::string msg = "Unknown transaction error: " + error_msg;
      BOOST_LOG_TRIVIAL(debug) << "Ethereum Adapter: parseTX_response, " << msg;
    }
  } catch (nlohmann::detail::exception &) {
    BOOST_LOG_TRIVIAL(debug)
        << "Ethereum Adapter: parseTX_response, error: Can not parse response "
           "from eth_sendTransaction, so unable "
           "to check mining result. Error parsing call response: "
        << read_buffer_call;
  }
}

auto EthereumAdapter::check_mining_result(std::string &transaction_ID)
    -> std::string {
  std::string response;
  size_t waited = 0;

  while ((waited + MINING_CHECK_INTERVAL) < this->max_waiting_time_) {
    std::this_thread::sleep_for(
        std::chrono::milliseconds(MINING_CHECK_INTERVAL));
    std::string transaction_param = "\"" + transaction_ID + "\"";
    std::string method = "eth_getTransactionByHash";
    response = call(transaction_param, method);

    try {
      nlohmann::json json_response = nlohmann::json::parse(response);

      if (!(json_response.at("result").at("blockNumber").is_null())) {
        std::stringstream msg;
        msg << "Mining took about " << waited << " ms";
        BOOST_LOG_TRIVIAL(debug)
            << "Ethereum Adapter: check_mining_result, " << msg.str();
        return response;
      }
    } catch (nlohmann::detail::exception &) {
      BOOST_LOG_TRIVIAL(debug)
          << "Ethereum Adapter: check_mining_result, Can't parse response"
          << response;
      // continue, so try again
    }

    waited += MINING_CHECK_INTERVAL;
  }

  return response;
}

auto EthereumAdapter::check_transaction_receipt(std::string &transaction_ID)
    -> bool {
  std::string method = "eth_getTransactionReceipt";
  std::string transaction_param = "\"" + transaction_ID + "\"";
  std::string response = call(transaction_param, method);

  try {
    nlohmann::json json_response = nlohmann::json::parse(response);
    BOOST_LOG_TRIVIAL(debug)
        << "Ethereum Adapter: check_transaction_receipt, Status: "
        << json_response.at("result").at("status").get<std::string>();
    if (json_response.at("result").at("status").get<std::string>() == "0x1") {
      return true;
    }
  } catch (nlohmann::detail::exception &e) {
    BOOST_LOG_TRIVIAL(debug)
        << "EthereumAdapter: check_transaction_receipt, Can't parse response"
        << e.what();
  }
  BOOST_LOG_TRIVIAL(debug)
      << "Ethereum Adapter: check_transaction_receipt, Response: " << response;
  return false;
}

auto EthereumAdapter::createRpcBatch(std::map<RpcParams, bool> batch,
                                     std::map<RpcParams, std::string> key_map)
    -> std::pair<std::map<std::string, std::string>,
                 std::map<std::string, std::string>> {
  std::string from_address = accountAddress_;
  std::map<RpcParams, bool>::iterator batch_iter;
  std::map<std::string, std::string> batch_transform;
  std::map<std::string, std::string> key_map_transform;

  for (batch_iter = batch.begin(); batch_iter != batch.end(); ++batch_iter) {
    // create intermediate structure for the params
    RpcParams intermed = batch_iter->first;

    intermed.from = from_address;

    if (intermed.to.empty()) {
      intermed.to = storedContractAddress_;
    }
    if (batch_iter->second) {
      intermed.gas = kEthereumGas;
    }

    // do we need to change the way how the nonce is updated? what is the real
    // logic behind that?
    if (intermed.method == "eth_sendTransaction") {
      intermed.nonce = ++nonce_;
    }

    std::string json = parse_params_to_json(intermed);

    const std::string quantity_tag = intermed.quantity_tag.empty()
                                         ? ""
                                         : ",\"" + intermed.quantity_tag + "\"";
    json += quantity_tag;

    // fill the batch_transform and key_map_transform maps with the respective
    // values
    batch_transform.emplace(json, intermed.method);

    std::map<RpcParams, std::string>::iterator key_iter;
    key_iter = key_map.find(batch_iter->first);
    key_map_transform.emplace(json, key_iter->second);
  }
  return std::pair<std::map<std::string, std::string>,
                   std::map<std::string, std::string>>(batch_transform,
                                                       key_map_transform);
}

auto EthereumAdapter::sendRpcBatch(std::map<std::string, std::string> batch,
                                   std::map<std::string, std::string> key_map)
    -> std::vector<std::string> {
  std::map<std::string, std::string>::iterator batch_iter;
  std::map<std::string, std::string>::iterator key_map_iter;
  std::map<std::string, std::string> json_tid_map;
  std::map<std::string, std::string>::iterator json_tid_iter;
  std::vector<std::string> output;

  std::string read_buffer;

  // iterate over all elements in the batch and initiate the transactions on the
  // blockchain
  for (batch_iter = batch.begin(); batch_iter != batch.end(); ++batch_iter) {
    std::string read_buffer_call;
    const std::string post_data = R"({"jsonrpc":"2.0","id":1,"method":")" +
                                  batch_iter->second + R"(","params":[)" +
                                  batch_iter->first + "]}";

    if (curl_ != nullptr) {
      struct curl_slist *headers = nullptr;
      headers = curl_slist_append(headers, "Content-Type: application/json");

      curl_easy_setopt(curl_, CURLOPT_HTTPHEADER, headers);
      curl_easy_setopt(curl_, CURLOPT_WRITEDATA, &read_buffer_call);
      curl_easy_setopt(curl_, CURLOPT_POSTFIELDS, post_data.c_str());
      CURLcode res = curl_easy_perform(curl_);

      if (res != CURLE_OK) {
        auto msg = "CURL perform() returned an error: " +
                   std::string(curl_easy_strerror(res));
        BOOST_LOG_TRIVIAL(debug) << "Ethereum Adapter: Call, " << msg;
      }
    }
    nlohmann::json json_response;
    parseTX_response(read_buffer_call, json_response);
    auto transaction_id = json_response["result"].get<std::string>();

    // create mapping between json and the transaction id
    json_tid_map.insert(
        std::pair<std::string, std::string>(batch_iter->first, transaction_id));
  }

  // check for all transactionIDs if the transaction has finished or not
  for (json_tid_iter = json_tid_map.begin();
       json_tid_iter != json_tid_map.end(); ++json_tid_iter) {
    read_buffer = check_mining_result(json_tid_iter->second);

    // if error, then add the corresponding key to the return vector
    if (!check_transaction_receipt(json_tid_iter->second)) {
      read_buffer = "error";

      key_map_iter = key_map.find(json_tid_iter->first);
      output.push_back(key_map_iter->second);
    }
  }
  return output;
}
/** @} */
