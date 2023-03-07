#include <chrono>
#include <iostream>
#include <memory>

#include <random>
#include <string>

#include "adapter_factory/adapter_factory.h"
#include "adapter_interface/adapter_interface.h"

using namespace std;

#define INPUT_SIZE 100

void table_commands();
auto read_batch() -> std::map<const BYTES, const BYTES>;
auto benchmark() -> bool;
auto random_string() -> std::string;
auto generate_batch(size_t batch_size, size_t tx_size) -> std::map<const BYTES, const BYTES>;
auto gen_random(int len) -> std::string;

std::string table_name;
std::string table_address;
std::string bc_type;
std::string net_config;

std::unique_ptr<BcAdapter> adapter = nullptr;

auto main(int argc, char *argv[]) -> int {
  cout << endl
       << "### Simple CLI for BlockchainAdapter ###" << endl
       << endl
       << "Enter a Blockchain Type: ";

  cin >> bc_type;

  // Create adapter instance
  adapter = AdapterFactory::create_adapter(AdapterFactory::getBC_TYPE(bc_type));
  if (!adapter) {
    cout << endl
         << "Unkown blockchain type: " << bc_type << endl
         << "Supported types are: [ETHEREUM, FABRIC, STUB]" << endl;
    return 1;
  }

  std::string config_path;
  if (argc > 1) {
    config_path = argv[1];
  } else {
    config_path = "./config.ini";
  }
  cout << "Adapter successfully initialized with config: " << config_path
       << endl;

  // Init adapter
  cout << endl << "Enter a network configuration:";
  getline(cin, net_config);
  getline(cin, net_config);

  if (!adapter->init(config_path, net_config)) {
    cout << "Initialize adapter failed!" << endl;
    return 1;
  }

  cout
      << endl
      << "Usage:" << endl
      << "\tcreate <table_name> \t\t\tCreates a new table with the given name."
      << endl
      << "\tload <table_name> <table_address> \tLoads the table with the given "
         "address."
      << endl
      << "\texit \t\t\t\t\tShuts down the adapter and exits the CLI." << endl
      << endl;

  while (true) {
    std::string cmd;
    std::array<string, 2> parameters = {"", ""};

    cout << endl << "Enter next command: ";
    std::string s_input;
    std::getline(std::cin, s_input);

    // Parsing input
    std::string delimiter = " ";
    size_t pos = 0;
    int i = 0;
    std::string token;
    while ((pos = s_input.find(delimiter)) != std::string::npos) {
      token = s_input.substr(0, pos);

      if (i == 0) {
        cmd = token;
      } else {
        parameters[i - 1] = token;
      }
      s_input.erase(0, pos + delimiter.length());
      i++;
    }
    switch (i) {
      case 0:
        cmd = s_input;
        break;
      case 1:
        parameters[0] = s_input;
        break;
      case 2:
        parameters[1] = s_input;
        break;
    }

    // Command execution
    if (cmd == "create") {
      table_name = parameters[0];
      if (adapter->create_table(table_name, table_address) != 0) {
        cout << "Can not create table " << table_name << endl;
        continue;
      }

      cout << "Create new table " << table_name << " with address "
           << table_address << endl;
    } else if (cmd == "load") {
      table_name = parameters[0];
      table_address = parameters[1];
      if (adapter->load_table(table_name, table_address) != 0) {
        cout << "Can not load table " << table_name << " with address "
             << table_address << endl;
        continue;
      }

      cout << "Load table " << table_name << " with address " << table_address
           << endl;
    } else if (cmd == "exit") {
      cout << "Shuting down!" << endl;
      break;
    } else {
      cout << "Wrong command: " << cmd << endl;
      continue;
    }
    table_commands();
  }

  adapter->shutdown();
  return 0;
}

void table_commands() {
  cout << endl
       << "Working on the table " << table_name << endl
       << endl
       << "Usage:" << endl
       << "\tput_batch \t\t\t\tInsert multiple entries. Insert '<key> <value>' "
          "and then return to insert the next key-value-pair."
          "\n\t\t\t\t\t\tTo stop the insertion, write 'STOP STOP' and return."
       << endl
       << "\tget <key> \t\t\t\tReads the given key from the table and returns "
          "its value."
       << endl
       << "\ttablescan \t\t\t\tReads all from the table." << endl
       << "\tremove <key> \t\t\t\tRemoves the key and its value from the table."
       << endl
       << "\tbenchmark \t\t\t\tRuns a short micro-benchmark thats loads some data into a table and reads it."
       << endl
       << "\tclose \t\t\t\t\tCloses the table." << endl
       << endl;

  while (true) {
    std::string cmd;
    std::array<string, 2> parameters = {"", ""};

    cout << endl << endl << "Enter next command: ";
    std::string s_input;
    std::getline(std::cin, s_input);

    cout << endl;

    // Parsing input
    std::string delimiter = " ";
    size_t pos = 0;
    int i = 0;
    std::string token;
    while ((pos = s_input.find(delimiter)) != std::string::npos) {
      token = s_input.substr(0, pos);
      if (i == 0) {
        cmd = token;
      } else {
        parameters[i - 1] = token;
      }
      s_input.erase(0, pos + delimiter.length());
      i++;
    }
    switch (i) {
      case 0:
        cmd = s_input;
        break;
      case 1:
        parameters[0] = s_input;
        break;
      case 2:
        parameters[1] = s_input;
        break;
    }

    // Command execution
    if (cmd == "put_batch") {
      std::map<const BYTES, const BYTES> batch = read_batch();
      adapter->put(batch);

      for (auto &it : batch) {
        std::string key =
            std::string((const char *)it.first.value, it.first.size);
        std::string value =
            std::string((const char *)it.second.value, it.second.size);
        cout << "Write key: " << key << " and value: " << value
             << " to the table." << endl;
      }
    } else if (cmd == "get") {
      std::string key = parameters[0];
      BYTES key_byte = BYTES(key);
      BYTES result_value;

      adapter->get(key_byte, result_value);
      std::string result_value_str =
          std::string((const char *)result_value.value, result_value.size);
      cout << "Result-Value: " << result_value_str << endl;
    } else if (cmd == "tablescan") {
      std::map<const BYTES, BYTES> results;
      adapter->get_all(results);
      if (results.empty()) {
        cout << "Table is empty" << endl;
      } else {
        cout << "Tablescan-Result:" << endl;
        for (auto &result : results) {
          std::string key =
              std::string((const char *)result.first.value, result.first.size);
          std::string value = std::string((const char *)result.second.value,
                                          result.second.size);
          cout << "Key: " << key << " Value: " << value << endl;
        }
      }
    } else if (cmd == "remove") {
      std::string key = parameters[0];
      BYTES key_byte = BYTES(key);
      adapter->remove(key_byte);
      cout << "Removed key: " << key << " from table." << endl;
    }
    if (cmd == "benchmark") {
      benchmark();
      break;

    } if (cmd == "close") {
      cout << "Close table!" << endl;
      cout << endl
           << "### Simple CLI for BlockchainAdapter ###" << endl
           << endl
           << "Usage:" << endl
           << "\tcreate <table_name> \t\t\tCreates a new table with the given "
              "name."
           << endl
           << "\tload <table_name> <table_address> \tLoads the table with the "
              "given "
              "address."
           << endl
           << "\texit \t\t\t\t\tShuts down the adapter and exits the CLI."
           << endl
           << endl;
      break;
    }
  }
}

auto read_batch() -> std::map<const BYTES, const BYTES> {
  std::map<const BYTES, const BYTES> batch;
  while (true) {
    std::string key;
    std::string value;
    BYTES key_byte;
    BYTES value_byte;
    cout << "Enter key:" << endl;
    std::cin >> key;
    cout << "Enter value:" << endl;
    std::cin >> value;
    key_byte = BYTES(key);
    value_byte = BYTES(value);
    if (key == "STOP") {
      break;
    }
    batch.emplace(key_byte, value_byte);
  }

  return batch;
}

auto benchmark() -> bool {
  const size_t batch_num = 300;
  const size_t batch_size = 1;
  const size_t tx_size = 100;

  std::vector<std::map<const BYTES, const BYTES>> data_batches;
  std::map<const BYTES, BYTES> result_map;
  std::vector<std::chrono::_V2::system_clock::time_point> put_start_timepoints;
  std::vector<std::chrono::_V2::system_clock::time_point> put_end_timepoints;
  std::vector<std::chrono::_V2::system_clock::time_point> read_start_timepoints;
  std::vector<std::chrono::_V2::system_clock::time_point> read_end_timepoints;
  vector<bool> rc;

  put_start_timepoints.reserve(batch_num);
  put_end_timepoints.reserve(batch_num);

  read_start_timepoints.reserve(batch_num);
  read_end_timepoints.reserve(batch_num);
  rc.reserve(batch_num);

  cout << endl << "Running micro-benchmark with " << batch_num << " batches of size " << batch_size << endl << endl;

  // Generate data
  cout << endl << "Generating data ...";
  auto start = std::chrono::high_resolution_clock::now();
  for (size_t i = 0; i < batch_num; i++) {
    std::map<const BYTES, const BYTES> batch = generate_batch(batch_size, tx_size);
    data_batches.emplace_back(batch);
  }
  auto elapsed = std::chrono::high_resolution_clock::now() - start;
  long long microseconds = std::chrono::duration_cast<std::chrono::microseconds>(
    elapsed).count();
  cout << " done! ("<< microseconds << " microseconds)" << endl << endl;

  // Load batch and measure read performance
  cout << endl << "Loading data ...";
  start = std::chrono::high_resolution_clock::now();
  for(auto batch:data_batches){
    // write next batch
    put_start_timepoints.emplace_back(std::chrono::high_resolution_clock::now());
    adapter->put(batch);
    put_end_timepoints.emplace_back(std::chrono::high_resolution_clock::now());

    // read all data
    read_start_timepoints.emplace_back(std::chrono::high_resolution_clock::now());
    rc.emplace_back(adapter->get_all(result_map));
    read_end_timepoints.emplace_back(std::chrono::high_resolution_clock::now());
  }
  elapsed = std::chrono::high_resolution_clock::now() - start;
  microseconds = std::chrono::duration_cast<std::chrono::microseconds>(
    elapsed).count();
  cout << " done! ("<< microseconds << " microseconds)" << endl << endl;

  for (size_t i = 0; i < batch_num; i++)
  {
    elapsed = put_end_timepoints.at(i) - put_start_timepoints.at(i);
    microseconds = std::chrono::duration_cast<std::chrono::microseconds>(
    elapsed).count();
    cout << "Batch " << i << " took "<< microseconds << " microseconds)" << endl;
  }

  for (size_t i = 0; i < batch_num; i++)
  {
    elapsed = read_end_timepoints.at(i) - read_start_timepoints.at(i);
    microseconds = std::chrono::duration_cast<std::chrono::microseconds>(
    elapsed).count();
    cout << "Reading data of " << i << " batches took "<< microseconds << " microseconds)" << endl;
  }

  return true;
}

auto generate_batch(size_t batch_size, size_t tx_size) -> std::map<const BYTES, const BYTES> {
  std::map<const BYTES, const BYTES> batch;
  for (size_t i = 0; i < batch_size; i++) {
    std::string key=random_string();
    std::string value=gen_random(tx_size);
    BYTES key_byte;
    BYTES value_byte;
    key_byte = BYTES(key);
    value_byte = BYTES(value);
    batch.emplace(key_byte, value_byte);
  }

  return batch;
}

auto random_string() -> std::string
{
     std::string str("0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz");

     std::random_device rd;
     std::mt19937 generator(rd());

     std::shuffle(str.begin(), str.end(), generator);

     return str.substr(0, 32);    // assumes 32 <- number of characters in str
}

auto gen_random(const int len) -> std::string {
    static const char kAlphanum[] =
        "0123456789"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
        "abcdefghijklmnopqrstuvwxyz";
    std::string tmp_s;
    tmp_s.reserve(len);

    for (int i = 0; i < len; ++i) {
        tmp_s += kAlphanum[rand() % (sizeof(kAlphanum) - 1)];
    }

    return tmp_s;
}