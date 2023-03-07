#ifndef TRUSTDBLE_COMMAND_H
#define TRUSTDBLE_COMMAND_H
#include <memory>
#include <string>

#include "boost/algorithm/string/trim.hpp"
#include <boost/algorithm/string.hpp>
#include <boost/lexical_cast.hpp>

#include "my_inttypes.h"
#include "my_psi_config.h"
#include "my_thread.h"

#include "adapter_factory/adapter_factory.h"
#include "rewrite_trustdble.h"
#include "trustdble_invite.h"

#include "bc_manager_factory/bc_manager_factory.h"
#include "bc_manager_interface/bc_manager_interface.h"

#include "my_sys.h"
#include "mysqld_error.h"
#include <mysql/components/my_service.h>
#include <mysql/components/services/log_builtins.h>

#include "blockchain/table_service.h"
#include "my_bitmap.h"
#include "sql/field.h"
#include "sql/sql_base.h"
#include "sql/sql_class.h"
#include "sql/sql_lex.h"
#include "sql/table.h"
#include "sql/transaction.h"
#include "storage/blockchain/include/blockchain/crypt_service.h"
#include <openssl/rand.h>
#include <iostream>
#include <netdb.h>
#include <sys/socket.h>
#include <arpa/inet.h>

// LOG-Tag for this class
#define LOG_TAG "blockchain"

using namespace trustdble;

// the parameter type represents if a parameter is positional : <param1> or
// keyvalue: "key"=<param1>
enum class ParameterType { positional, keyvalue };

/**
* @brief Struct that is representing a parameter used in a query
* It stores name, type, default_value and position of a parameter.
*/
struct PARSE_PARAMETER {
  ParameterType type;     // positional or keyvalue
  std::string identifier; // "0" for positional, "type" for keyvalue
  std::string
      default_value;  // undefined --> REQUIRED, defined --> DEFAULT_VALUE
  std::string result; // Parsing result
};


/**
 * @brief Reduce double spaces with the single space
 *
 * @param str String to modify
 *
 * @return Modified string
 */
static std::string remove_double_spaces(std::string str) {
  const std::string double_spaces = "  ";
  const std::string single_space = " ";
  // find double spaces
  std::string::size_type pos = str.find(double_spaces);

  while (pos != std::string::npos) {
    // replace both spaces with single space
    str.replace(pos, 2, single_space);
    // start searching again
    pos = str.find(double_spaces, pos);
  }

  return str;
}

/**
 * @brief Trim parameter string, remove double spaces, replace “ =” and “= “
 * with “=”
 *
 * @param str String to modify
 */
static void tidy_spaces(std::string &str) {
  // clean parameter string from multiple white spaces
  boost::algorithm::trim(str); // remove multiple white spaces from both sides
  //boost::replace_all(str, "(", " ("); // add space before ( symbol --commented because function like COUNT(*) are also rewritten and that breaks the query
  str =
      remove_double_spaces(str); // replace double spaces with the single space
  boost::replace_all(str, " =", "="); // remove space before eq. symbol
  boost::replace_all(str, "= ", "="); // remove space after eq. symbol
}

/**
* @brief enumeration class that is representing all the shared trustdble commands
*
*/
enum class TrustdbleSharedCommand {
  create_database,
  load_database,
  drop_database,
  create_table,
  drop_table,
  load_table,
  load_tables,
  load_shards,
  create_table_invite,
  add_table_invite,
  repartition_database,
  partition_tables,
  create_invite,
  add_invite,
  add_data_contract,
  create_data_contract,
  encrypt_data_contract
};
/**
 * @brief Struct that is representing all the config parameters needed for
 * executing the trustdble commands
 *
 */
struct CONFIG_SET {
  char *config_dependency_path;
  char *config_configuration_path;
  char *config_invite_path;
  char *config_server_number;
};

class TrustdbleCommand {
protected:
  std::map<std::string, PARSE_PARAMETER>
      parameter_Set; // parameter_set used for parsing the parameters
  TrustdbleSharedCommand shared_command;
  std::string parameter_string; // query string with parameters
  MYSQL_THD thd;

public:
  TrustdbleCommand(TrustdbleSharedCommand command, std::string &parameter,
                   std::map<std::string, PARSE_PARAMETER> parameter_set,
                   MYSQL_THD thread);

  /**
   * @brief parse the parameters of the query
   *
   * @return 0 when successfull otherwise 1
   *
   */
  auto parse()
      -> int; // parse all parameters from parameter_string into parameter_set

  /**
   * @brief executes the trustble command
   *
   * @param command_queries query to be executed
   * @param config config data needed for the execution
   *
   * @return 0 when successfull otherwise 1
   *
   */
  auto execute(std::string &command_queries, CONFIG_SET config)
      -> int; // execution of the specific command
};

#endif // TRUSTDBLE_COMMAND_H
