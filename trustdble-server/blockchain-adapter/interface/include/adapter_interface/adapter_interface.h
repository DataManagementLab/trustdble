/** @defgroup group50 adapter_interface
 *  @ingroup group5
 *  @{
 */
#ifndef ADAPTER_INTERFACE_H
#define ADAPTER_INTERFACE_H

#include <boost/property_tree/ptree.hpp>
#include <iomanip>
#include <string>
#include <vector>

namespace pt = boost::property_tree;

/**
 * @brief Struct that is representing a pair of value in bytes and it's size.
 * It is used to send key/value with it's size to adapter methods
 */
struct BYTES {
  //! The array of bytes stored in the BYTES object
  unsigned char *value;
  //! The array's length
  size_t size;

  /**
   * @brief Constructs a BYTES object from an array of bytes
   *
   * @param send_value The array of bytes as an unsigned char array
   * @param send_size The arrays length
   */
  BYTES(unsigned char *send_value, size_t send_size) {
    value = new unsigned char[send_size];
    memcpy(value, send_value, send_size);
    size = send_size;
  }
  /**
   * @brief Constructs a BYTES object of only one byte
   */
  BYTES() {
    value = new unsigned char[1];
    size = 1;
    value[0] = '\0';
  }
  /**
   * @brief Constructs a BYTES object from a string interpreted as
   * an array of chars
   *
   * @param send_value The string to be stored in the BYTES object
   */
  BYTES(const std::string &send_value) {
    size = send_value.size();
    value = new unsigned char[size];
    memcpy(value,
           reinterpret_cast<unsigned char *>(
               const_cast<char *>(send_value.c_str())),
           size);
  }
  /**
   * @brief Copy constructor for BYTES objects. Initializes a new BYTES
   * object with the bytes of the other object by copying the underlying
   * array to a second memory location
   *
   * @param that The BYTES object of which bytes are copied
   */
  BYTES(const BYTES &that) {
    size = that.size;
    value = new unsigned char[size];
    memcpy(value, that.value, size);
  }
  //! Destructor
  ~BYTES() { delete[] value; }

  /**
   * @brief Assignment operator for BYTES objects. Fills the object that's being
   * assigned to with the bytes of the other objects by copying the underlying
   * array to a second memory location
   *
   * @param that The BYTES object of which bytes are copied
   *
   * @return The object that's being assigned to
   */
  auto operator=(const BYTES &that) -> BYTES & {
    if (this == &that) {
      return *this;
    }
    size = that.size;
    unsigned char *local_value = new unsigned char[size];
    memcpy(local_value, that.value, size);
    delete[] value;
    value = local_value;
    return *this;
  }
};

/**
 * @brief Checks for equality between two BYTES objects by first checking
 * if they have the same number of bytes and then checking if all their bytes
 * are the same
 *
 * @param lhs The BYTES object on the left side of the == operator
 * @param lhs The BYTES object on the right side of the == operator
 *
 * @return a bool indicating if the objects are equal
 */
inline auto operator==(const BYTES &lhs, const BYTES &rhs) -> bool {
  bool equal = lhs.size == rhs.size;
  if (equal) {
    for (size_t i = 0; i < lhs.size; i++) {
      equal = lhs.value[i] == rhs.value[i];
      if (!equal) {
        return equal;
      }
    }
  }
  return equal;
}

/**
 * @brief Checks if the BYTES object to the left of the < operator is
 * smaller than the BYTES object to the right of the < operator. First
 * the object containing less bytes is considered smaller. Second
 * the object to the left is smaller if its array of bytes represents
 * a smaller number
 *
 * @param lhs The BYTES object on the left side of the < operator
 * @param lhs The BYTES object on the right side of the < operator
 *
 * @return a bool indicating if the object on the left is smaller
 */
inline auto operator<(const BYTES &lhs, const BYTES &rhs) -> bool {
  if (lhs.size < rhs.size) {
    return true;
  }
  if (lhs.size == rhs.size) {
    for (size_t i = 0; i < lhs.size; i++) {
      if (lhs.value[i] < rhs.value[i]) {
        return true;
      }
      if (lhs.value[i] > rhs.value[i]) {
        return false;
      }
    }
    return false;
  }
  return false;
}

/**
 * @brief Interface definition to be used by storage engine to communicate with
 * concrete blockchain technology adapter, like Ethereum, Fabric, ...
 *
 */
class BcAdapter {
 public:
  //! Destructor
  virtual ~BcAdapter() = default;

  /**
   * @brief Initialize adapter with full configuration from file including
   * adapter and network configurations
   *
   * @param config_path Path to configuration
   * @return true if init was successful, false if init returned an error
   */
  virtual auto init(const std::string &config_path) -> bool = 0;

  /**
   * @brief Initialize adapter with adapter configuration from file and network
   * configuration
   *
   * @param config_path Path to static adapter configuration file
   * @param network_config Network configuration as JSON formatted string
   * @return true if init was successful, false if init returned an error
   */
  virtual auto init(const std::string &config_path,
                    const std::string &network_config) -> bool = 0;

  /**
   * @brief Uninitialize adapter and close connection to blockchain
   *
   * @return True if shutdown was successful, otherwise false
   */
  virtual auto shutdown() -> bool = 0;

  /**
   * @brief Put a batch of key-value pairs into the blockchain
   *
   * @param batch Batch including multiple key-value pairs; succesfully inserted
   * key-value pairs are removed from the batch
   *
   * @return status code (0 on success, 1 on failure batch contains remaining
   * key-value pairs)
   */
  virtual auto put(std::map<const BYTES, const BYTES> &batch) -> int = 0;

  /**
   * @brief Get a value of a key-value pair from the blockchain
   *
   * @param key Key of the pair
   * @param result Reference to store read value
   *
   * @return status code (0 on success, 1 on failure)
   */
  virtual auto get(const BYTES &key, BYTES &result) -> int = 0;

  /**
   * @brief Gets all key-value pairs from the blockchain
   *
   * @param results Reference of a vector of tuples to store read pairs
   *
   * @return status code (0 on success, 1 on failure)
   */
  virtual auto get_all(std::map<const BYTES, BYTES> &results) -> int = 0;

  /**
   * @brief Remove a key value pair from the blockchain
   *
   * @param key Key of the pair
   *
   * @return status code (0 on success, 1 on failure)
   */
  virtual auto remove(const BYTES &key) -> int = 0;
  /**
   * @brief Create a table (contract) in the blockchain
   *
   * @param name The name of the table to deploy in the blockchain
   * @param tableAddress The address of the table in the blockchain, this can be
   * used to load the table in future
   * @return int returns 0 on success, 1 on failure
   */
  virtual auto create_table(const std::string &name, std::string &tableAddress)
      -> int = 0;

  /**
   * @brief Connect to an existing table (contract) in the blockchain
   *
   * @param name The name of the table that should be loaded
   * @param tableAddress The address (connection information) that is used to
   * connect to the table (interpreted by adapters individually)
   * @return int returns 0 on success, 1 on failure
   */
  virtual auto load_table(const std::string &name,
                          const std::string &tableAddress) -> int = 0;

  /**
   * @brief Drops a table and deletes all entries from blockchain
   *
   * @return int returns 0 on success, 1 on failure
   */
  virtual auto drop_table() -> int = 0;

  /**
   * @brief Produces a hex encoded representation of an array of bytes
   *
   * @param data The array of bytes as an unsigned char array
   * @param length The arrays length
   *
   * @return Returns the hex encoded representation in a string
   */
  static auto byte_array_to_hex(const unsigned char *data, unsigned length)
      -> std::string {
    std::stringstream ss;
    ss << std::hex;
    unsigned i = 0;
    for (; i < length; i++) {
      ss << std::setw(2) << std::setfill('0') << (int)(data[i]);
    }
    return ss.str();
  }

  /**
   * @brief Produces an array of bytes from a hex encoded representation
   *
   * @param in The string containing the hex encoded bytes
   * @param data Return parameter containing the array of bytes as an
   * unsigned char array
   *
   * @return Returns void
   */
  static void hex_to_byte_array(const std::string &in, unsigned char *data) {
    if ((in.length() % 2) != 0) {
      throw std::runtime_error("String is not valid length ...");
    }
    size_t cnt = in.length() / 2;

    for (size_t i = 0; cnt > i; ++i) {
      int s = 0;
      std::stringstream ss;
      ss << std::hex << in.substr(i * 2, 2);
      ss >> s;

      data[i] = (unsigned char)(s);
    }
  }
};

#endif  // ADAPTER_INTERFACE_H
/** @} */