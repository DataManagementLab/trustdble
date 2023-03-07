#ifndef TRUSTDBLE_TRANSACTION
#define TRUSTDBLE_TRANSACTION

#include <string>
#include <vector>
#include <unordered_map>
#include <map>
#include <cstring>
#include "adapter_factory/adapter_factory.h"
using namespace std;

namespace trustdble {

/**
 * @brief Enum to distinguish between different statements
 *
 */
enum class STATEMENT_TYPE{WRITE, REMOVE};

/**
 * @brief Struct that stores a single statement.
 *
 * @param type Type of the statement (write or remove)
 * @param tablename The name of the table to which this statement will be applied
 * @param key The key that this statement targets
 * @param value The value of the write statement. Empty if it is remove statement
 *
 */
struct STATEMENT{
  STATEMENT_TYPE type;
  std::string tablename;
  BYTES key;
  BYTES value;
};

/**
 * @brief Transaction class that is used in the blockchain storage engine to store all information while executing database statements.
 * When a transaction is startet the storage engine creates a new object of this class and adds all statements that are processed to it
 * and stores for each accessed table a copy in a table cache that is used until commit. When committing the list of statements is executed
 * and applied to the blockchain. When rolling back the transaction it is cleared and nothing is applied to the blockchain.
 *
 */
class Transaction {
  public:
    auto init() -> int;
    /**
     * @brief Adds a table to the table cache of this transaction
     *
     * @param tablename Name of the table
     * @param table_map Table as map of strings (key, value)
     * @return 0 if success
     */
    auto addTable(const std::string &tablename, std::map<BYTES, BYTES> &table_map) -> int;
    /**
     * @brief Adds a write statement to the statement list
     *
     * @param tablename Name of the table that statement belongs to
     * @param key The key of the write statement
     * @param value The value of the write statement
     * @return 0 if success
     */
    auto addWrite(const std::string &tablename, BYTES &key,  BYTES &value) -> int;
    /**
     * @brief Adds a remove statement to the statement list
     *
     * @param tablename Name of the table that statement belongs to
     * @param key The key of the remove statement
     * @return 0 if success
     */
    auto addRemove(const std::string &tablename, const BYTES &key) -> int;

    // List of statements of the transaction
    std::vector<STATEMENT> statements;
    // Cache holding all used tables of the transaction.
    std::unordered_map<std::string, std::map<BYTES, BYTES>> table_cache;
    // Counter of locks
    ulong lock_count=0;
};

} // namespace trustdble

#endif // TRUSTDBLE_TRANSACTION