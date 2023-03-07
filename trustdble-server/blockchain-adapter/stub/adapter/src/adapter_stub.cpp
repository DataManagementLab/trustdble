/*! \addtogroup group20
 *  @{
 */
#include "adapter_stub/adapter_stub.h"

#include <boost/filesystem.hpp>
#include <map>

////////////////////// Stub IMPLEMENTATION ///////////////////////////

// Constructor
StubAdapter::StubAdapter() { BOOST_LOG_TRIVIAL(debug) << "stub: Constructor"; }

// Destructor
StubAdapter::~StubAdapter() = default;

auto StubAdapter::init(const std::string &config_path) -> bool {
  config_.init(config_path);

  // the directory(network) should be already exists
  if (!boost::filesystem::is_directory(config_.data_path())) {
    BOOST_LOG_TRIVIAL(debug) << "STUBAdatper: network " << config_.data_path()
                             << " doesn't exist, init Adapter failed";
    return false;
  }

  blockNumber_.init(config_.data_path() + "/blocknumber.txt",
                    config_.blocksize());
  return true;
}

auto StubAdapter::init(const std::string &config_path,
                       const std::string &network_config) -> bool {
  // init adapter with config_path and parse network_config to add to
  // adapter config
  bool result = config_.init(config_path);

  if (network_config.empty()) {
    return false;
  }

  result &= config_.set_network_config(network_config);

  // the directory(network) should be already exists
  if (!boost::filesystem::is_directory(config_.data_path())) {
    BOOST_LOG_TRIVIAL(debug) << "STUBAdatper: network " << config_.data_path()
                             << " doesn't exists, init Adapter failed";

    return false;
  }

  blockNumber_.init(config_.data_path() + "/blocknumber.txt",
                    config_.blocksize());

  return result;
}

auto StubAdapter::shutdown() -> bool {
  this->tableName_ = "";
  return true;
}

auto StubAdapter::put(std::map<const BYTES, const BYTES> &batch) -> int {
  std::string filename_old = config_.data_path() + "/" + tableName_ + ".txt";
  std::string filename_new =
      config_.data_path() + "/" + tableName_ + "_tmp.txt";
  std::string line;

  // Open old file
  std::ifstream old_file(filename_old);
  if (!old_file.is_open()) {
    BOOST_LOG_TRIVIAL(debug)
        << "stub: PUT, Failed to open File '" << filename_old << "'!";
    return 1;
  }

  // Open new file
  std::ofstream new_file(filename_new, std::fstream::out | std::fstream::app);
  if (!new_file.is_open()) {
    BOOST_LOG_TRIVIAL(debug)
        << "stub: PUT, Failed to open File '" << filename_new << "'!";
    old_file.close();
    return 1;
  }

  // create iterator for the batch
  std::map<const BYTES, const BYTES>::iterator it;

  // Copy old file to new file
  while (getline(old_file, line)) {
    unsigned char *key = new unsigned char[line.size() / 2];
    hex_to_byte_array(line, key);
    BYTES bytes_line(key, line.size() / 2);
    delete[] key;
    // check if key is already in  the batch
    it = batch.find(bytes_line);
    if (it == batch.end()) {
      new_file << line << "\n";
      getline(old_file, line);
      new_file << line << "\n";
    } else {
      getline(old_file, line);
    }
  }

  // append the key-value pairs included in the batch to the end of
  // the file
  for (it = batch.begin(); it != batch.end(); ++it) {
    std::string key = byte_array_to_hex(it->first.value, it->first.size);
    std::string value = byte_array_to_hex(it->second.value, it->second.size);
    new_file << key << "\n";
    new_file << value << "\n";
  }

  new_file.close();
  old_file.close();

  // Remove old file and rename new file
  if (std::remove(filename_old.c_str()) != 0) {
    BOOST_LOG_TRIVIAL(debug)
        << "stub: PUT, Removing of file '" << filename_old << "' failed!";
    return 1;
  }
  if (rename(filename_new.c_str(), filename_old.c_str()) != 0) {
    BOOST_LOG_TRIVIAL(debug)
        << "stub: PUT, Renaming of file '" << filename_new << "' failed!";
    return 1;
  }

  // Put done
  BOOST_LOG_TRIVIAL(debug) << "stub: PUT, success";
  return 0;
}

auto StubAdapter::put(const std::string &key, const std::string &value,
                      const std::string &signature, unsigned int transactionId,
                      unsigned int nodeId, unsigned int blockTimeout) -> int {
  if (!hasValidSignature(signature, key, transactionId, nodeId, blockTimeout)) {
    return 1;
  }

  if (hasLockTimedOut(blockTimeout)) {
    return 1;
  }

  // Extended this function to be compatible with the batching put method
  std::map<const BYTES, const BYTES> map;
  map.emplace(BYTES(key), BYTES(value));
  return put(map);
}

auto StubAdapter::get(const BYTES &key, BYTES &result) -> int {
  std::string line;

  std::ifstream myfile(config_.data_path() + "/" + tableName_ + ".txt");

  if (myfile.is_open()) {
    while (getline(myfile, line)) {
      unsigned char *line_byte = new unsigned char[line.length() / 2];
      hex_to_byte_array(line, line_byte);
      BYTES bytes_line(line_byte, line.length() / 2);
      delete[] line_byte;
      if (key == bytes_line) {
        BOOST_LOG_TRIVIAL(debug) << "stub: GET, Success";
        getline(myfile, line);
        unsigned char *line_byte_end = new unsigned char[line.length() / 2];
        hex_to_byte_array(line, line_byte_end);
        myfile.close();
        result = BYTES(line_byte_end, line.length() / 2);
        delete[] line_byte_end;
        return 0;
      }
    }
    myfile.close();
    BOOST_LOG_TRIVIAL(debug)
        << "stub: GET, No value for key %s found!"
        << std::string(reinterpret_cast<char *>(key.value), key.size);
    return 1;
  }
  BOOST_LOG_TRIVIAL(debug)

      << "stub: GET, Failed to open File " << config_.data_path().c_str() << "/"
      << tableName_.c_str() << ".txt";

  return 1;
}

auto StubAdapter::get(const std::string &key, std::string &result,
                      const std::string &signature, unsigned int transactionId,
                      unsigned int nodeId, unsigned int blockTimeout) -> int {
  if (!hasValidSignature(signature, key, transactionId, nodeId, blockTimeout)) {
    return 1;
  }

  if (hasLockTimedOut(blockTimeout)) {
    return 1;
  }

  BYTES bytes_key(key);
  BYTES bytes_result(result);
  return get(bytes_key, bytes_result);
}

auto StubAdapter::get_all(std::map<const BYTES, BYTES> &results) -> int {
  std::string key;
  std::string value;
  std::ifstream myfile(config_.data_path() + "/" + tableName_ + ".txt");

  if (myfile.is_open()) {
    while (getline(myfile, key)) {
      unsigned char *key_byte = new unsigned char[key.length() / 2];
      hex_to_byte_array(key, key_byte);
      getline(myfile, value);
      unsigned char *value_byte = new unsigned char[value.length() / 2];
      hex_to_byte_array(value, value_byte);
      results.emplace(BYTES(key_byte, key.length() / 2),
                      BYTES(value_byte, value.length() / 2));
      delete[] value_byte;
      delete[] key_byte;
    }
    myfile.close();
    BOOST_LOG_TRIVIAL(debug) << "stub: GET_ALL, Success";
    return 0;
  }
  BOOST_LOG_TRIVIAL(debug)

      << "stub: GET_ALL, Failed to open File " << config_.data_path().c_str()
      << "/" << tableName_.c_str() << ".txt!";

  return 1;
}

auto StubAdapter::remove(const BYTES &key) -> int {
  std::string line;
  bool key_found = false;

  std::ifstream old_file(config_.data_path() + "/" + tableName_ + ".txt");
  std::ofstream new_file(config_.data_path() + "/" + tableName_ + "_tmp.txt",
                         std::fstream::out | std::fstream::app);

  if (old_file.is_open() && new_file.is_open()) {
    while (getline(old_file, line)) {
      unsigned char *line_byte = new unsigned char[line.length() / 2];
      hex_to_byte_array(line, line_byte);
      BYTES bytes_line(line_byte, line.length() / 2);
      delete[] line_byte;
      if (key == bytes_line) {
        getline(old_file, line);
        key_found = true;
      } else {
        new_file << line << "\n";
        getline(old_file, line);
        new_file << line << "\n";
      }
    }
    old_file.close();
    new_file.close();

    if (std::remove(
            (config_.data_path() + "/" + tableName_ + ".txt").c_str()) == 0) {
      // logg("Failed to delete File " + config.data_path() + "" + tableName +
      // ".txt!","remove");

      if (rename((config_.data_path() + "/" + tableName_ + "_tmp.txt").c_str(),
                 (config_.data_path() + "/" + tableName_ + ".txt").c_str()) !=
          0) {
        // logg("Renaming failed!", "remove");
        return 1;
      }
    }

    if (key_found) {
      BOOST_LOG_TRIVIAL(debug) << "stub: REMOVE, Success";
      return 0;
    }
    BOOST_LOG_TRIVIAL(debug) << "stub: REMOVE, failed due to key not found";
    return 1;
  }
  BOOST_LOG_TRIVIAL(debug)

      << "stub: REMOVE, Failed to open File" << config_.data_path().c_str()
      << "/" << tableName_.c_str() << ".txt";

  return 1;
}

auto StubAdapter::remove(const std::string &key, const std::string &signature,
                         unsigned int transactionId, unsigned int nodeId,
                         unsigned int blockTimeout) -> int {
  if (!hasValidSignature(signature, key, transactionId, nodeId, blockTimeout)) {
    return 1;
  }

  if (hasLockTimedOut(blockTimeout)) {
    return 1;
  }

  return remove(BYTES(key));
}

auto StubAdapter::create_table(const std::string &name,
                               std::string &tableAddress) -> int {
  this->tableName_ = name;
  std::ofstream myfile(this->config_.data_path() + "/" + tableName_ + ".txt",
                       std::fstream::out | std::fstream::app);

  if (myfile.is_open()) {
    myfile.close();
  }
  tableAddress = this->config_.data_path() + "/" + tableName_ + ".txt";
  return 0;
}

auto StubAdapter::load_table(const std::string &name,
                             const std::string &tableAddress) -> int {
  (void)tableAddress;
  this->tableName_ = name;
  std::ifstream myfile(this->config_.data_path() + "/" + tableName_ + ".txt");
  if (myfile.is_open()) {
    myfile.close();
  } else {
    return 1;
  }
  return 0;
}

auto StubAdapter::drop_table() -> int {
  if (std::remove((config_.data_path() + "/" + tableName_ + ".txt").c_str()) !=
      0) {
    BOOST_LOG_TRIVIAL(debug)
        << "stub: DROP_TABLE, Failed to delete File"
        << config_.data_path().c_str() << "/" << tableName_.c_str();
  } else {
    BOOST_LOG_TRIVIAL(debug)
        << "stub: DROP_TABLE, Delete File " << config_.data_path().c_str()
        << "/" << tableName_.c_str();
  }

  return 0;
}

auto StubAdapter::getCurrentBlockNumber() -> unsigned int {
  return blockNumber_.get();
}

auto StubAdapter::hasValidSignature(const std::string &signature,
                                    const std::string &key,
                                    unsigned int transactionId,
                                    unsigned int nodeId,
                                    unsigned int blockTimeout) -> bool {
  (void)signature;
  std::string plaintext = key + "#" + std::to_string(transactionId) + "#" +
                          std::to_string(nodeId) + "#" +
                          std::to_string(blockTimeout);
  return true;
}

auto StubAdapter::hasLockTimedOut(unsigned int blockTimeout) -> bool {
  return blockTimeout > getCurrentBlockNumber();
}
/** @} */