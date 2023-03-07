/*! \addtogroup group33
 *  @{
 */
#include "adapter_fabric/client_fabric.h"

#include <iostream>
#include <map>
#include <string>
#include <utility>

#include "../extern/go_client/libclient.hpp"

// Helper methods

/**
 * @brief Transform a map of key-value pairs into a GoString containing
 * a JSON representation of the map
 *
 * @param bytes_map a map of key-value pairs
 *
 * @return key-value pairs represented as a GoString in JSON format
 */
static auto map_to_json_go_string(std::map<const BYTES, const BYTES>& bytes_map)
    -> GoString {
  nlohmann::json json_object = nlohmann::json::object();
  for (const auto& iter : bytes_map) {
    std::string key =
        BcAdapter::byte_array_to_hex(iter.first.value, iter.first.size);
    std::string value =
        BcAdapter::byte_array_to_hex(iter.second.value, iter.second.size);
    json_object[key] = value;
  }
  std::string serialized_map = json_object.dump();
  char* serialized_map_bytes = new char[serialized_map.length()];
  memcpy(serialized_map_bytes, serialized_map.c_str(), serialized_map.length());
  GoString serialized_go_map = {serialized_map_bytes,
                                (long)serialized_map.length()};
  return serialized_go_map;
}

/**
 * @brief Transform a JSON representation of a map of key-value pairs
 * into a map
 *
 * @param serialized_map key-value pairs represented as string in JSON format
 *
 * @return a map of key-value pairs
 */
static auto json_string_to_map(char* serialized_map)
    -> std::map<const BYTES, BYTES> {
  std::map<const BYTES, BYTES> bytes_map = std::map<const BYTES, BYTES>();
  nlohmann::json json_object = nlohmann::json::parse(serialized_map);
  for (const auto& iter : json_object.items()) {
    // BYTES implementation leads to unecessary allocation here
    // should BYTES be changed or should we not use the constructor here?
    auto key_length = iter.key().length() / 2;
    unsigned char* key_bytes = new unsigned char[key_length];
    BcAdapter::hex_to_byte_array(iter.key(), key_bytes);
    BYTES key = BYTES(key_bytes, iter.key().length() / 2);
    delete[] key_bytes;

    auto value_length = iter.value().get<std::string>().length() / 2;
    unsigned char* value_bytes = new unsigned char[value_length];
    BcAdapter::hex_to_byte_array(iter.value().get<std::string>(), value_bytes);
    BYTES value = BYTES(value_bytes, value_length);
    delete[] value_bytes;
    bytes_map[key] = value;
  }
  return bytes_map;
}

/**
 * @brief Tranform a list of byte arrays into a GoString containing a JSON
 * representation of the list
 *
 * @param bytes_list a list of byte arrays
 *
 *  @return list of byte arrays as a GoString in JSON format
 */
static auto list_to_json_go_string(std::list<BYTES>& bytes_list) -> GoString {
  nlohmann::json json_array = nlohmann::json::array();
  for (const auto& bytes : bytes_list) {
    std::string hex = BcAdapter::byte_array_to_hex(bytes.value, bytes.size);
    json_array.push_back(hex);
  }
  std::string serialized_list = json_array.dump();
  char* serialized_list_bytes = new char[serialized_list.length()];
  memcpy(serialized_list_bytes, serialized_list.c_str(),
         serialized_list.length());
  GoString serialized_go_list = {serialized_list_bytes,
                                 (long)serialized_list.length()};
  return serialized_go_list;
}

/**
 * @brief Return a GoString containing a JSON representation of an empty
 * object
 *
 * @return an empty object represented as a GoString in JSON format
 */
static auto empty_object_json_go_string() -> GoString {
  std::string empty_json_object = "{}";
  char* serialized_map_bytes = new char[empty_json_object.length()];
  memcpy(serialized_map_bytes, empty_json_object.c_str(),
         empty_json_object.length());
  GoString go_empty_json_object = {serialized_map_bytes,
                                   (long)empty_json_object.length()};
  return go_empty_json_object;
}

/**
 * @brief Transform bytes into a GoString containing a hex encoded
 * representation of the byte array
 *
 * @param in an array of bytes in a BYTES object
 *
 * @return GoString containing the bytes encoded as hex
 */
static auto bytes_to_go_string(const BYTES& in) -> GoString {
  std::string hex_representation =
      BcAdapter::byte_array_to_hex(in.value, in.size);
  char* hex_representation_bytes = new char[hex_representation.length()];
  memcpy(hex_representation_bytes, hex_representation.c_str(),
         hex_representation.length());
  GoString go_hex_representation = {hex_representation_bytes,
                                    (long)hex_representation.length()};
  return go_hex_representation;
}

/**
 * @brief Transform a string into a GoString
 *
 * @param in a C++ string
 *
 * @return a GoString
 */
static auto string_to_go_string(const std::string& in) -> GoString {
  char* string_bytes = new char[in.length()];
  memcpy(string_bytes, in.c_str(), in.length());
  GoString go_string = {string_bytes, (long)in.length()};
  return go_string;
}

// Constructor
FabricClient::FabricClient() = default;

// Destructor
FabricClient::~FabricClient() = default;

// Mapped contract methods
auto FabricClient::init(std::string channel_name, std::string contract_name,
                        std::string msp_id, std::string cert_path,
                        std::string key_path, std::string tls_cert_path,
                        std::string peer_endpoint, std::string gateway_peer,
                        std::string table_name) -> int {
  if (isInitializied_) {
    if (close() != 0) {
      return 1;
    }
  }

  this->channel_name_ = std::move(channel_name);
  this->contract_name_ = std::move(contract_name);
  this->msp_id_ = std::move(msp_id);
  this->cert_path_ = std::move(cert_path);
  this->key_path_ = std::move(key_path);
  this->tls_cert_path_ = std::move(tls_cert_path);
  this->peer_endpoint_ = std::move(peer_endpoint);
  this->gateway_peer_ = std::move(gateway_peer);
  this->table_name_ = std::move(table_name);
  GoString go_msp_id = {this->msp_id_.c_str(), (long)this->msp_id_.length()};
  GoString go_cert_path = {this->cert_path_.c_str(),
                           (long)this->cert_path_.length()};
  GoString go_key_path = {this->key_path_.c_str(),
                          (long)this->key_path_.length()};
  GoString go_tls_cert_path = {this->tls_cert_path_.c_str(),
                               (long)this->tls_cert_path_.length()};
  GoString go_peer_endpoint = {this->peer_endpoint_.c_str(),
                               (long)this->peer_endpoint_.length()};
  GoString go_gateway_peer = {this->gateway_peer_.c_str(),
                              (long)this->gateway_peer_.length()};
  GoString go_channel_name = {this->channel_name_.c_str(),
                              (long)this->channel_name_.length()};
  GoString go_contract_name = {this->contract_name_.c_str(),
                               (long)this->contract_name_.length()};

  int status_code =
      Init(go_channel_name, go_contract_name, go_msp_id, go_cert_path,
           go_key_path, go_tls_cert_path, go_peer_endpoint, go_gateway_peer);

  isInitializied_ = true;

  return status_code;
}

auto FabricClient::put(std::map<const BYTES, const BYTES>& batch) -> int {
  GoString go_gateway_peer = {this->gateway_peer_.c_str(),
                              (long)this->gateway_peer_.length()};
  GoString go_table_name = {this->table_name_.c_str(),
                            (long)this->table_name_.length()};
  GoString key_value_pairs = map_to_json_go_string(batch);
  GoString function = string_to_go_string("put");
  int status_code =
      Write(key_value_pairs, function, go_table_name, go_gateway_peer);
  delete[] key_value_pairs.p;
  delete[] function.p;
  return status_code;
}

auto FabricClient::get(const BYTES& key, BYTES& value) -> int {
  GoString go_gateway_peer = {this->gateway_peer_.c_str(),
                              (long)this->gateway_peer_.length()};
  GoString go_table_name = {this->table_name_.c_str(),
                            (long)this->table_name_.length()};
  GoString go_key = bytes_to_go_string(key);
  GoString function = string_to_go_string("get");
  Read_return go_result =
      Read(go_key, function, go_table_name, go_gateway_peer);
  delete[] go_key.p;
  delete[] function.p;
  if (go_result.r2 != 0) {
    return 1;
  }
  unsigned char* value_bytes = new unsigned char[go_result.r1 / 2];
  BcAdapter::hex_to_byte_array(go_result.r0, value_bytes);
  value = BYTES(value_bytes, go_result.r1 / 2);
  return 0;
}

auto FabricClient::getAll(std::map<const BYTES, BYTES>& values) -> int {
  GoString go_gateway_peer = {this->gateway_peer_.c_str(),
                              (long)this->gateway_peer_.length()};
  GoString go_table_name = {this->table_name_.c_str(),
                            (long)this->table_name_.length()};
  GoString empty_json_object = empty_object_json_go_string();
  GoString function = string_to_go_string("getAll");
  Read_return result =
      Read(empty_json_object, function, go_table_name, go_gateway_peer);
  delete[] empty_json_object.p;
  delete[] function.p;
  if (result.r2 != 0) {
    return 1;
  }
  values = json_string_to_map(result.r0);
  return 0;
}

auto FabricClient::remove(std::list<BYTES>& batch) -> int {
  GoString go_gateway_peer = {this->gateway_peer_.c_str(),
                              (long)this->gateway_peer_.length()};
  GoString go_table_name = {this->table_name_.c_str(),
                            (long)this->table_name_.length()};

  GoString go_keys = list_to_json_go_string(batch);

  GoString function = string_to_go_string("delete");
  int status_code = Write(go_keys, function, go_table_name, go_gateway_peer);
  delete[] go_keys.p;
  delete[] function.p;
  return status_code;
}

auto FabricClient::close() -> int {
  isInitializied_ = false;
  GoString go_gateway_peer = {this->gateway_peer_.c_str(),
                              (long)this->gateway_peer_.length()};
  GoString go_contract_name = {this->contract_name_.c_str(),
                               (long)this->contract_name_.length()};
  int status_code = Close(go_contract_name, go_gateway_peer);
  this->channel_name_ = "";
  this->contract_name_ = "";
  this->msp_id_ = "";
  this->cert_path_ = "";
  this->key_path_ = "";
  this->tls_cert_path_ = "";
  this->peer_endpoint_ = "";
  this->gateway_peer_ = "";
  this->table_name_ = "";
  return status_code;
}

// Public methods
auto FabricClient::isInit() const -> bool { return isInitializied_; }
/** @} */