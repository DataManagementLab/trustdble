#include "trustdble_command.h"
#include <errno.h>
#include "trustdble_utils/general_helpers.h"

using namespace blockchain_manager;
using namespace trustdble;
namespace pt = boost::property_tree;

static bool plugin_init = false;

/**
 * @brief Struct that represents docker container information
 */
struct CONTAINER_INFO {
  std::string container_name;
  std::string rpc_port;
  std::string peer_port;
  std::string join_ip;
};

/**
 * @brief Format string in insert query
 *
 * @param input_str[in] Input string
 * @param field_schema[in] Field schema
 *
 * @return Output string
 */
static std::string format_query(std::string input_str,
                                std::string field_schema) {
  boost::algorithm::to_lower(field_schema);
  // str to return
  std::string output_str = "";
  // check if field type is in (VARCHAR, CHAR, TEXT, DATE)
  if ((field_schema.find("varchar") != std::string::npos) ||
      (field_schema.find("char") != std::string::npos) ||
      (field_schema.find("text") != std::string::npos) ||
      (field_schema.find("date") != std::string::npos)) {
    output_str = "'" + input_str + "'";  // format as text, add single quotes
  } else {
    output_str = input_str;  // format as number
  }
  return output_str;
}

/**
 * @brief Create name for the meta/data chain network (container)
 *
 * @param network_type Network type
 * @param db_name Database name
 * @param config_server_number Serever number
 * @param shard_number Shard number (optional parameter)
 *
 * @return Network (container) name
 */
static std::string create_network_name(std::string network_type,
                                       std::string db_name,
                                       char *config_server_number,
                                       int shard_number = 1) {
  // network name prefix
  const std::string NETWORK_NAME_PREFIX_META = "meta";
  const std::string NETWORK_NAME_PREFIX_DATA = "data";
  // network (container) name
  std::string network_name = "";

  // name of container for META blockchain network
  if (network_type == NETWORK_NAME_PREFIX_META) {
    network_name.append(NETWORK_NAME_PREFIX_META);  // "metaBC"
    network_name.append(config_server_number);      // + server_number
    network_name.append(db_name);                   // + db_name
  }
  // name of container for DATA blockchain network
  else if (network_type == NETWORK_NAME_PREFIX_DATA) {
    network_name.append(NETWORK_NAME_PREFIX_DATA);      // "dataBC"
    network_name.append(config_server_number);          // + server_number
    network_name.append(db_name);                       // + db_name
    network_name.append("_shard_");                     // + "_shard_"
    network_name.append(std::to_string(shard_number));  // + shard_number
  } else {
    LogPluginErr(ERROR_LEVEL, ER_REWRITER_QUERY_FAILED,
                 "create_network_name(), network_type should be meta or data");
    return "";
  }

  return network_name;
}

/**
 * @brief Parse schema of table
 *
 * @param[in] col_num Column number
 * @param[in] table_schema Table Schema
 *
 * @return schema of field
 */
static std::string parse_table_schema(const int col_num,
                                      const std::string &table_schema) {
  // example: get schema for col_num = 1 (<lastName VARCHAR(255)>)
  std::string str_to_parse =
      table_schema;  // id INT, lastName VARCHAR(255), firstName VARCHAR(255)
  // columns separated by delim_symbol
  const std::string delim_symbol = ", ";  // symbol {', '}

  // find position of first symbol of extracted substring (first non-space
  // symbol is in a query string)
  size_t pos_start = 0;
  int pos = 0;
  while (pos < col_num) {
    pos_start = str_to_parse.find(delim_symbol) +
                1;  // id INT, {*}lastName VARCHAR(255), firstName VARCHAR(255)
    str_to_parse = str_to_parse.substr(
        pos_start);  // lastName VARCHAR(255), firstName VARCHAR(255)
    pos++;
  }

  // find position of last symbol of extracted substring
  size_t pos_end = str_to_parse.find(
      delim_symbol);  // lastName VARCHAR(255){*}, firstName VARCHAR(255)
  if (pos_end == std::string::npos) {
    pos_end = str_to_parse.length();  // lastName VARCHAR(255){*}
  }

  std::string parameter_value =
      str_to_parse.substr(0, pos_end);  // lastName VARCHAR(255)
  return parameter_value;
}

/**
 * @brief Parse parameter value from query string using it's position to find it
 *
 * @param parameter_position Parameter position (first position is 0)
 * @param query_string Query string
 *
 * @return Parsed parameter value
 */
static std::string parse_positional_parameter(
    const int parameter_position, const std::string &query_string,
    const std::string delimitter = " ") {
  // example: get parameter_value for parameter_position = 1 (<par_A>)
  std::string str_to_parse =
      query_string;  // <db_name> <par_A> type=<bc_type> shards=<num_shards>
                     // encrypt=<encryption_status>
  // position parameters separated by delim_symbol
  const std::string delim_symbol = " ";  // symbol {' '}

  // find position of first symbol of extracted substring (first non-space
  // symbol is in a query string)
  size_t pos_start = 0;
  int pos = 0;
  // std::string sub_str_to_parse;
  // std::string lower_str_to_parse;
  while (pos < parameter_position) {
    if (str_to_parse.find(delim_symbol) == std::string::npos) {
      LogPluginErr(ERROR_LEVEL, ER_REWRITER_QUERY_FAILED,
                   "parse_positional_parameter(), cannot find parameter in a "
                   "query string");
      return "";
    }

    pos_start = str_to_parse.find(delim_symbol) +
                1;  // <db_name> {*}<par_A> type=<bc_type> shards=<num_shards>
                    // encrypt=<encryption_status>
    str_to_parse = str_to_parse.substr(
        pos_start);  // <par_A> type=<bc_type> shards=<num_shards>
                     // encrypt=<encryption_status>
    pos++;
  }

  // find position of last symbol of extracted substring
  size_t pos_end = str_to_parse.find(
      delimitter);  // <par_A>{*} type=<bc_type> shards=<num_shards>
                    // encrypt=<encryption_status>
  if (pos_end == std::string::npos) {
    pos_end = str_to_parse.length();  // <par_A>{*}
  }

  std::string parameter_value = str_to_parse.substr(0, pos_end);  // <par_A>

  if (parameter_value.find("=") != std::string::npos) {
    LogPluginErr(ERROR_LEVEL, ER_REWRITER_QUERY_FAILED,
                 "parse_positional_parameter(), parameter is not positional, "
                 "buy keyvalue");
    return "";
  }
  return parameter_value;
}

/**
 * @brief Parse parameter value from query string using it's key (parameter
 * name) to find it
 *
 * @param parameter_key Parameter key
 * @param query_string Query string
 *
 * @return Parsed parameter value
 */
static std::string parse_keyvalue_parameter(
    const std::string parameter_key, const std::string &query_string,
    const std::string delimitter = " ") {
  // example: get parameter_value for parameter_key = type
  std::string str_to_parse =
      query_string;  // <db_name> <par_A> type=<bc_type> num_shards=<num_shards>
                     // encrypt=<encryption_status>
  std::string lower_str_to_parse = str_to_parse;
  boost::algorithm::to_lower(lower_str_to_parse);
  // find position of first symbol of extracted substring
  const std::string equal_symbol = "=";                          // symbol {'='}
  const std::string key_to_find = parameter_key + equal_symbol;  // type=

  if (lower_str_to_parse.find(key_to_find) == std::string::npos) {
    LogPluginErr(
        ERROR_LEVEL, ER_REWRITER_QUERY_FAILED,
        "parse_keyvalue_parameter(), cannot find parameter in a query string");
    return "";
  }

  size_t pos_start = lower_str_to_parse.find(
      key_to_find);  // <db_name> <par_A> {*}type=<bc_type>
                     // num_shards=<num_shards> encrypt=<encryption_status>
  pos_start +=
      key_to_find
          .length();  // <db_name> <par_A> type={*}<bc_type>
                      // num_shards=<num_shards> encrypt=<encryption_status>
  str_to_parse =
      str_to_parse.substr(pos_start);  // <bc_type> num_shards=<num_shards>
                                       // encrypt=<encryption_status>

  // find position of last symbol of extracted substring
  size_t pos_end =
      str_to_parse.find(delimitter);  // <bc_type>{*} num_shards=<num_shards>
                                      // encrypt=<encryption_status>
  if (pos_end == std::string::npos) {
    pos_end = str_to_parse.length();  // <bc_type>{*}
  }

  std::string parameter_value = str_to_parse.substr(0, pos_end);  // <bc_type>
  return parameter_value;
}

/**
 * @brief Get create-statement of specific table
 *
 * @param tablename Name of the table
 * @param stmt String to store statement
 *
 * @return 0 if successful, otherwise 1
 */
static int getStmt(const std::string &tablename, std::string &stmt,
                   const char *shared_db) {
  std::vector<std::tuple<std::string, std::string>> list;
  if (getMetaData(list, shared_db) != 0) {
    return 1;
  }

  for (auto entry : list) {
    if (tablename.compare(std::get<0>(entry)) == 0) {
      stmt = CREATE_TABLE.UNSHARED + std::string(shared_db) + "." + tablename +
             " (" + std::get<1>(entry) + ")" + ENGINE_STRING + ";";
      return 0;
    }
  }
  stmt = "";
  return 0;
}

/**
 * @brief Get create-statement of specific data_table
 *
 * @param tablename Name of the table
 * @param stmt String to store statement
 *
 * @return 0 if successful, otherwise 1
 */
static int getDataTable(const std::string &tablename, std::string &stmt,
                        const char *shared_db) {
  DBUG_PRINT(LOG_TAG, ("Rewrite-Plugin: getDataTable called"));
  std::vector<std::tuple<std::string, std::string>> list;
  if (getMetaDataForSharedDataTable(list, shared_db) != 0) {
    return 1;
  }
  int data_chain_id = 0;
  // get partition_id = table_name + '/' + data_chain_id
  const std::string partition_id =
      tablename + '/' + std::to_string(data_chain_id);
  for (auto entry : list) {
    if (partition_id.compare(std::get<0>(entry)) == 0) {
      stmt = CREATE_TABLE.UNSHARED + "if not exists " + std::string(shared_db) +
             "." + tablename + " (" + std::get<1>(entry) + ")" + ENGINE_STRING +
             ";";
      DBUG_PRINT(LOG_TAG,
                 ("Rewrite-Plugin: getDataTable, stmt = %s", stmt.c_str()));
      return 0;
    }
  }
  stmt = "";
  return 0;
}

TrustdbleCommand::TrustdbleCommand(
    TrustdbleSharedCommand command, std::string &parameter,
    std::map<std::string, PARSE_PARAMETER> parameter_set, MYSQL_THD thread) {
  shared_command = command;
  parameter_string = parameter;
  thd = thread;
  parameter_Set = parameter_set;
}

/**
 * @brief parse the parameters of the query
 *
 * @return 0 when successfull otherwise 1
 *
 */
auto TrustdbleCommand::parse() -> int {
  for (auto const &entry : parameter_Set) {
    PARSE_PARAMETER parameter = entry.second;
    if (parameter.type == ParameterType::positional) {
      if (entry.first == "schema") {
        parameter.result = parse_positional_parameter(
            boost::lexical_cast<int>(parameter.identifier), parameter_string,
            ";");
      } else {
        parameter.result = parse_positional_parameter(
            boost::lexical_cast<int>(parameter.identifier), parameter_string);
      }

    } else if (parameter.type == ParameterType::keyvalue) {
      tidy_spaces(parameter_string);
      if (entry.first == "invite_string" or entry.first == "path" or
          entry.first == "public_key") {
        parameter.result = parse_keyvalue_parameter(parameter.identifier,
                                                    parameter_string, ";");
      } else {
        parameter.result =
            parse_keyvalue_parameter(parameter.identifier, parameter_string);
      }
    }
    if (parameter.result == "") {
      parameter.result = parameter.default_value;
    }
    parameter_Set[entry.first] = parameter;
  }
  return 0;
}
/**
 * @brief executes the trustble command
 *
 * @param command_queries query to be executed
 * @param config config data needed for the execution
 *
 * @return 0 when successfull otherwise 1
 *
 */
int TrustdbleCommand::execute(std::string &command_queries, CONFIG_SET config) {
  switch (shared_command) {
    case TrustdbleSharedCommand::create_database: {
      DBUG_PRINT(LOG_TAG, ("trustdble_command_method_call: create_database"));
      // obj representing a shared database
      trustdble::SHARED_DATABASE shared_database;

      // get value of parameter database name
      PARSE_PARAMETER db_name = parameter_Set["database_name"];
      DBUG_PRINT(LOG_TAG,
                 ("Rewrite-Plugin: db_name :: %s", db_name.result.c_str()));
      // check db_name is valid
      if (db_name.result == "") {
        LogPluginErr(ERROR_LEVEL, ER_REWRITER_QUERY_FAILED,
                     "Invalid database name!");
        return 1;
      }
      // fetching the shared database
      if (trustdble::getSharedDatabase(shared_database,
                                       db_name.result.c_str()) == 0) {
        command_queries = CREATE_DATABASE.UNSHARED + db_name.result;
        DBUG_PRINT(LOG_TAG, ("database already exists %s",
                             shared_database.name.c_str()));
        return 1;
      }

      // Check if an other database being created.
      // If not, start creating, otherweise refused.
      switch (try_and_set_SHM(plugin_init)) {
        case 1:  // ready for create database
          plugin_init = true;
          DBUG_PRINT(LOG_TAG, ("start create database"));
          break;
        case 0:  // not ready
          my_error(ER_CANT_CREATE_DB, MYF(0), db_name.result.c_str(), errno,
                   "Refused to create database because another database is "
                   "being created");
          return 1;
        default:  // error
          my_error(ER_CANT_CREATE_DB, MYF(0), db_name.result.c_str(), errno,
                   "Plugin read shared memory faild");
          return 1;
      }

      // get value of parameter blockchain type
      PARSE_PARAMETER bc_type = parameter_Set["type"];
      DBUG_PRINT(LOG_TAG,
                 ("Rewrite-Plugin: bc_type :: %s", bc_type.result.c_str()));
      // check bc_type is valid
      if (!(bc_type.result.compare("ETHEREUM") == 0 ||
            bc_type.result.compare("STUB") == 0 ||
            bc_type.result.compare("FABRIC") == 0)) {
        LogPluginErr(ERROR_LEVEL, ER_REWRITER_QUERY_FAILED,
                     "Invalid blockchain type!");
        resetSHM();
        return 1;
      }
      // get value of parameter encryption status
      PARSE_PARAMETER encrypt = parameter_Set["encryption"];
      DBUG_PRINT(LOG_TAG,
                 ("Rewrite-Plugin: encrypt :: %s", encrypt.result.c_str()));
      // check encryption status is valid
      if (!(encrypt.result.compare("true") == 0 ||
            encrypt.result.compare("false") == 0)) {
        LogPluginErr(ERROR_LEVEL, ER_REWRITER_QUERY_FAILED,
                     "Invalid encryption status!");
        resetSHM();
        return 1;
      }

      // get value of parameter number of shards (data-chains)
      PARSE_PARAMETER numshards = parameter_Set["shards"];
      int num_shards;
      try {
        num_shards = boost::lexical_cast<int>(numshards.result);
      } catch (boost::bad_lexical_cast const &e) {  // invalid input
        LogPluginErr(ERROR_LEVEL, ER_REWRITER_QUERY_FAILED,
                     "Invalid number of shards, can't parse parameter string!");
        resetSHM();
        return 1;
      }
      // check num_shards is valid (>0)
      if (!(num_shards > 0)) {
        LogPluginErr(
            ERROR_LEVEL, ER_REWRITER_QUERY_FAILED,
            "Invalid number of shards, number of shards should be > 0! %s",
            numshards.result.c_str());
        resetSHM();
        return 1;
      }
      DBUG_PRINT(LOG_TAG, ("Rewrite-Plugin: num_shards :: %d", num_shards));

      command_queries = CREATE_DATABASE.UNSHARED + db_name.result;

      // Set database name
      shared_database.name = db_name.result;
      // Set bctype
      shared_database.bc_type = bc_type.result;
      // Set encryption key and iv
      if ((encrypt.result.compare("true") == 0)) {
        unsigned char *db_encryption_key = new unsigned char[KEY_SIZE];
        unsigned char *db_iv = new unsigned char[IV_SIZE];
        RAND_bytes(db_encryption_key, KEY_SIZE);
        RAND_bytes(db_iv, IV_SIZE);
        shared_database.encryption_key =
            charArrayToHex(db_encryption_key, KEY_SIZE);
        shared_database.iv = charArrayToHex(db_iv, IV_SIZE);
        delete[] db_encryption_key;
        delete[] db_iv;
      }

      // collection to store connection information
      std::vector<CONTAINER_INFO> containers;

      // start meta blockchain network
      std::unique_ptr<BcManagerNetworkConfig> meta_network_config = nullptr;

      // network name
      std::string network_name = "";
      // name of container for META blockchain network
      network_name = create_network_name("meta", db_name.result,
                                         config.config_server_number);
      DBUG_PRINT(LOG_TAG, ("Rewrite-Plugin: Define network name %s",
                           network_name.c_str()));

      std::unique_ptr<BcManager> manager = BcManagerFactory::create_manager(
          BcManagerFactory::getBC_TYPE(bc_type.result));

      if (manager == nullptr) {
        LogPluginErr(ERROR_LEVEL, ER_REWRITER_QUERY_FAILED,
                     "ManagerFactory creation failed due to invalid bctype!");
        resetSHM();
        return 1;
      }

      // used when bctype is STUB, ETHEREUM or FABRIC
      manager->init(config.config_configuration_path);
      // check network creation is successful
      if (manager->create_network(network_name, meta_network_config) == 1) {
        LogPluginErr(ERROR_LEVEL, ER_REWRITER_QUERY_FAILED,
                     "MetaChain creation failed!");
        resetSHM();
        return 1;
      }

      // Format and set meta_chain_config
      shared_database.meta_chain_config = meta_network_config->get_json();
      if (bc_type.result.compare("ETHEREUM") == 0) {
        // parse connection information
        std::stringstream ss_data;
        ss_data << meta_network_config->get_json();
        pt::ptree net_config;
        pt::read_json(ss_data, net_config);
        // store connection information
        std::string rpc_port = net_config.get<std::string>("Network.rpc-port");
        std::string peer_port =
            net_config.get<std::string>("Network.peer-port");
        std::string join_ip = net_config.get<std::string>("Network.join-ip");
        containers.push_back({network_name, rpc_port, peer_port, join_ip});
      }

      manager.reset();
      DBUG_PRINT(LOG_TAG,
                 ("Rewrite-Plugin: Creating meta-table for database %s",
                  db_name.result.c_str()));

      // collection to store data_network_config information about data-chains
      std::vector<std::string> data_chains_network_config;
      // start data blockchain network
      // create multiple data chains
      for (int shard_number = 0; shard_number < num_shards; shard_number++) {
        DBUG_PRINT(
            LOG_TAG,
            ("Rewrite-Plugin: Create multiple data chains, shard_number %d",
             shard_number));
        std::unique_ptr<BcManagerNetworkConfig> data_network_config = nullptr;

        // name of container for DATA blockchain network
        network_name = create_network_name(
            "data", db_name.result, config.config_server_number, shard_number);
        DBUG_PRINT(LOG_TAG, ("Rewrite-Plugin: Define network name %s",
                             network_name.c_str()));

        manager = BcManagerFactory::create_manager(
            BcManagerFactory::getBC_TYPE(bc_type.result));
        if (manager == nullptr) {
          LogPluginErr(ERROR_LEVEL, ER_REWRITER_QUERY_FAILED,
                       "ManagerFactory creation failed due to invalid bctype!");
          resetSHM();
          return 1;
        }

        manager->init(config.config_configuration_path);
        // check network creation is successful
        if (manager->create_network(network_name, data_network_config) != 0) {
          LogPluginErr(ERROR_LEVEL, ER_REWRITER_QUERY_FAILED,
                       "DataChain creation failed!");
          resetSHM();
          return 1;
        }

        // set data chain config
        std::string str_data_network_config = data_network_config->get_json();
        boost::replace_all(str_data_network_config, " ",
                           "");  // remove all whitespace characters
        data_chains_network_config.push_back(str_data_network_config);
        DBUG_PRINT(LOG_TAG, ("Rewrite-Plugin: create multiple data chains, "
                             "str_data_network_config %s",
                             str_data_network_config.c_str()));

        if (bc_type.result.compare("ETHEREUM") == 0) {
          // get connection information
          std::stringstream ss_data;
          ss_data << data_network_config->get_json();
          pt::ptree net_config;
          pt::read_json(ss_data, net_config);
          // store connection information
          std::string rpc_port =
              net_config.get<std::string>("Network.rpc-port");
          std::string peer_port =
              net_config.get<std::string>("Network.peer-port");
          std::string join_ip = net_config.get<std::string>("Network.join-ip");
          containers.push_back({network_name, rpc_port, peer_port, join_ip});
        }

        manager.reset();
      }

      command_queries.append(";");
      // add statements to query for insert into database meta table
      std::stringstream meta_chain_config_ss;
      meta_chain_config_ss << quoted(shared_database.meta_chain_config);

      std::string insert_stmt = "";
      std::string create_stmt = "";

      if (bc_type.result.compare("ETHEREUM") == 0) {
        // store docker container and blockchain shard information in
        // trustdble.docker_containers table
        insert_stmt = "INSERT INTO trustdble.docker_containers ";
        insert_stmt.append(
            "(container_name, database_name, blockchain_type, rpc_port, "
            "peer_port, join_ip) VALUES ");
        for (auto &container : containers) {
          insert_stmt.append(
              "('" + container.container_name + "','" + shared_database.name +
              "','" + shared_database.bc_type + "','" + container.rpc_port +
              "','" + container.peer_port + "','" + container.join_ip + "')");
          if (&container != &containers.back()) {
            insert_stmt.append(", ");
          } else {
            insert_stmt.append(";");
          }
        }
        command_queries.append(insert_stmt);
      }

      // add statements to query for insert into database meta table
      insert_stmt =
          "insert into trustdble.shared_databases (database_name, "
          "blockchain_type, meta_table_address, meta_chain_config, "
          "encryption_key, iv) values ('" +
          shared_database.name + "','" + shared_database.bc_type + "','" +
          shared_database.meta_address + "', " + meta_chain_config_ss.str() +
          ",'" + shared_database.encryption_key + "','" + shared_database.iv +
          "'" + ");";
      command_queries.append(insert_stmt);

      // use database <db_name>
      create_stmt = " USE " + db_name.result + ";";
      command_queries.append(create_stmt);

      // create meta-table
      create_stmt = " create table " + db_name.result + "." + META_TABLE_NAME +
                    " " + META_TABLE_SCHEMA + "" + ENGINE_STRING + ";";
      command_queries.append(create_stmt);

      // create shared table data_chains on meta-chain
      create_stmt = " create table " + db_name.result + "." +
                    META_TABLE_DATA_CHAINS_NAME + " " +
                    META_TABLE_DATA_CHAINS_SCHEMA + ENGINE_STRING + ";";
      command_queries.append(create_stmt);

      // create shared table shared_tables on meta-chain
      create_stmt = " create table " + db_name.result + "." +
                    SHARED_TABLES_NAME + " " + SHARED_TABLES_SCHEMA +
                    ENGINE_STRING + ";";
      command_queries.append(create_stmt);

      // create key_store
      create_stmt = " create table " + db_name.result + "." +
                    KEY_STORE_TABLE_NAME + " " + KEY_STORE_TABLE_SCHEMA + ";";
      command_queries.append(create_stmt);

      // insert into data_chains table information about data-chains (shards)
      insert_stmt =
          " USE " + db_name.result + ";" + " INSERT INTO data_chains VALUES ";
      for (int shard_number = 0; shard_number < num_shards; shard_number++) {
        insert_stmt.append("(" + std::to_string(shard_number) + ",'" +
                           data_chains_network_config[shard_number] + "')");
        if (shard_number < num_shards - 1) {
          insert_stmt.append(", ");
        } else {
          insert_stmt.append(";");
        }
      }
      command_queries.append(insert_stmt);

      // update database_state to CREATED state
      std::string update_stmt =
          " update trustdble.shared_databases "
          "set database_state='CREATED' "
          "where database_name = '" +
          db_name.result + "'" + ";";
      command_queries.append(update_stmt);

      break;
    }
    case TrustdbleSharedCommand::repartition_database: {
      DBUG_PRINT(LOG_TAG,
                 ("trustdble_command_method_call: repartition_database"));
      command_queries = "";

      // get database name
      std::string database_name = parameter_Set["database_name"].result;
      DBUG_PRINT(LOG_TAG, ("repartition_database: database_name = %s",
                           database_name.c_str()));
      // check database name is valid
      if (database_name == "") {
        LogPluginErr(ERROR_LEVEL, ER_REWRITER_QUERY_FAILED,
                     "Invalid database name!");
        command_queries =
            "SIGNAL  SQLSTATE '42S01' SET MYSQL_ERRNO='1050', MESSAGE_TEXT = "
            "'repartition_database: Invalid database name!';";
        return 1;
      }

      // get number of shards (data-chains) to partition database
      PARSE_PARAMETER numshards = parameter_Set["shards"];
      int num_shards;
      try {
        num_shards = boost::lexical_cast<int>(numshards.result);
      } catch (boost::bad_lexical_cast const &e) {
        // invalid input
        LogPluginErr(ERROR_LEVEL, ER_REWRITER_QUERY_FAILED,
                     "partition_database: can't parse parameter string! "
                     "parameter shards (number of shards) is missing or has "
                     "wrong type (expected INT)!");
        command_queries =
            "SIGNAL  SQLSTATE '42S01' SET MYSQL_ERRNO='1050', MESSAGE_TEXT = "
            "'partition_database: can't parse parameter string! parameter "
            "shards (number of shards) is missing or has wrong type (expected "
            "INT)!';";
        return 1;
      }
      // check number of shards is valid (>0)
      if (!(num_shards > 0)) {
        LogPluginErr(
            ERROR_LEVEL, ER_REWRITER_QUERY_FAILED,
            "Invalid number of shards, number of shards should be > 0! %s",
            numshards.result.c_str());
        command_queries =
            "SIGNAL  SQLSTATE '42S01' SET MYSQL_ERRNO='1050', MESSAGE_TEXT = "
            "'repartition_database: Invalid number of shards, number of shards "
            "should be > 0!';";
        return 1;
      }
      DBUG_PRINT(LOG_TAG, ("partition_database: num_shards = %d", num_shards));

      trustdble::SHARED_DATABASE shared_database;
      const char *shared_db;
      shared_db = database_name.c_str();

      // fetching the shared database
      if (trustdble::getSharedDatabase(shared_database, shared_db) == 0) {
        DBUG_PRINT(
            LOG_TAG,
            ("repartition_database: Shared database: %s, %s, %s, %s",
             shared_database.name.c_str(), shared_database.bc_type.c_str(),
             shared_database.meta_address.c_str(),
             shared_database.meta_chain_config.c_str()));
      } else {
        LogPluginErr(ERROR_LEVEL, ER_REWRITER_QUERY_FAILED,
                     "partition_database: database does not exist!");
        command_queries =
            "SIGNAL SQLSTATE '42S01' SET MYSQL_ERRNO='1050', MESSAGE_TEXT = "
            "'partition_database: database does not exist!';";
        return 1;
      }

      // get number of shards which are in database
      int initial_num_shards = getNumShards(database_name);
      DBUG_PRINT(LOG_TAG, ("partition_database: initial number of shards = %d",
                           initial_num_shards));

      // get difference between initial number of shards and number of shards to
      // partition to
      int delta_num_shards = num_shards - initial_num_shards;
      DBUG_PRINT(LOG_TAG, ("partition_database: delta_num_shards = %d",
                           delta_num_shards));

      // error: partition database into the same number of shards that database
      // is already partitioned into
      if (delta_num_shards == 0) {
        LogPluginErr(ERROR_LEVEL, ER_REWRITER_QUERY_FAILED,
                     "Invalid number of shards = %d, database is already "
                     "partitioned into that number of shards",
                     num_shards);
        command_queries =
            "SIGNAL  SQLSTATE '42S01' SET MYSQL_ERRNO='1050', MESSAGE_TEXT = "
            "'repartition_database: invalid number of shards, database is "
            "already partitioned into that number of shards';";
        return 1;
      }

      DBUG_PRINT(LOG_TAG, ("partition_database: partition database from %d "
                           "into %d number of shards",
                           initial_num_shards, num_shards));

      // vector to store table_name information for all data tables (tables on
      // data chains)
      std::vector<std::string> list_of_tables;
      get_list_of_tables(list_of_tables, database_name.c_str());

      // copy each data table to local tables in local DB trustdble
      for (auto table : list_of_tables) {
        DBUG_PRINT(LOG_TAG,
                   ("partition_database: copy table = %s data", table.c_str()));
        // table scheme
        std::string table_schema = "";
        // get schema of table
        get_table_schema(table, table_schema, shared_db);
        // create local temporary table
        std::string tmp_table = "tmp_" + table;
        std::string create_stmt = " CREATE TABLE IF NOT EXISTS trustdble." +
                                  tmp_table + " " + "(" + table_schema + ")" +
                                  ";";
        command_queries.append(create_stmt);

        // vector to store (PK) key for all rows in table
        std::vector<std::string> list_of_keys;
        if (getListOfKeys(list_of_keys, table.c_str(), shared_db) != 0) {
          return 1;
        }

        // don't copy table if NO data in table
        if (!list_of_keys.empty()) {
          // table data
          std::vector<std::vector<std::string>> table_data;
          // copy table data
          copy_table(shared_db, table.c_str(), table_data);
          for (const auto &row : table_data) {
            for (const auto &cell : row) {
              DBUG_PRINT(LOG_TAG,
                         ("partition_database: table_cell = %s", cell.c_str()));
            }
          }

          // insert each row into tmp local table
          for (const auto &row : table_data) {
            std::string insert_row =
                " insert into trustdble." + tmp_table + " values ";
            std::string table_row = "(";
            int col_num = 0;
            for (const auto &cell : row) {
              // get schema for the cell
              std::string field_schema =
                  parse_table_schema(col_num, table_schema);
              DBUG_PRINT(LOG_TAG, ("partition_database: field_schema = %s",
                                   field_schema.c_str()));
              // add single quotes for the cell, if type in NOT INT
              std::string formated_cell = format_query(cell, field_schema);
              DBUG_PRINT(LOG_TAG, ("partition_database: formated_cell = %s",
                                   formated_cell.c_str()));
              table_row.append(formated_cell);
              if (&cell != &row.back()) {
                table_row.append(",");
              } else {
                table_row.append(")");
              }
              col_num++;  // next cell -> next table_column
            }
            insert_row.append(table_row);
            insert_row.append(";");
            DBUG_PRINT(LOG_TAG, ("partition_database: insert_row = %s",
                                 insert_row.c_str()));
            command_queries.append(insert_row);
          }

          // delete each row in table
          std::string delete_table_data =
              " delete from " + database_name + "." + table + ";";
          command_queries.append(delete_table_data);
        }
      }

      // BC manager
      std::unique_ptr<BcManager> manager;

      // add shards
      if (delta_num_shards > 0) {
        // vector to store connection information
        std::vector<CONTAINER_INFO> containers;
        // vector to store data_network_config information about data-chains
        std::vector<std::string> data_chains_network_config;

        // create new networks (multiple data chains)
        for (int shard_number = initial_num_shards; shard_number < num_shards;
             shard_number++) {
          DBUG_PRINT(LOG_TAG, ("partition_database: Create multiple data "
                               "chains, shard_number = %d",
                               shard_number));
          std::unique_ptr<BcManagerNetworkConfig> data_network_config = nullptr;

          // network name for DATA blockchain network
          std::string network_name = create_network_name(
              "data", database_name, config.config_server_number, shard_number);
          DBUG_PRINT(LOG_TAG, ("partition_database: define network name %s",
                               network_name.c_str()));

          manager = BcManagerFactory::create_manager(
              BcManagerFactory::getBC_TYPE(shared_database.bc_type));
          if (manager == nullptr) {
            LogPluginErr(
                ERROR_LEVEL, ER_REWRITER_QUERY_FAILED,
                "ManagerFactory creation failed due to invalid bctype!");
            return 1;
          }

          manager->init(config.config_configuration_path);
          // check network creation is successful
          if (manager->create_network(network_name, data_network_config) != 0) {
            LogPluginErr(ERROR_LEVEL, ER_REWRITER_QUERY_FAILED,
                         "DataChain creation failed!");
            return 1;
          }

          // set data chain config
          std::string str_data_network_config = data_network_config->get_json();
          boost::replace_all(str_data_network_config, " ",
                             "");  // remove all whitespace characters
          data_chains_network_config.push_back(str_data_network_config);
          DBUG_PRINT(LOG_TAG, ("partition_database: create multiple data "
                               "chains, str_data_network_config %s",
                               str_data_network_config.c_str()));

          if (shared_database.bc_type.compare("ETHEREUM") == 0) {
            // get connection information
            std::stringstream ss_data;
            ss_data << data_network_config->get_json();
            pt::ptree net_config;
            pt::read_json(ss_data, net_config);
            // store connection information
            std::string rpc_port =
                net_config.get<std::string>("Network.rpc-port");
            std::string peer_port =
                net_config.get<std::string>("Network.peer-port");
            std::string join_ip =
                net_config.get<std::string>("Network.join-ip");
            DBUG_PRINT(LOG_TAG,
                       ("repartition_database: connection information for ETH: "
                        "%s, %s, %s",
                        rpc_port.c_str(), peer_port.c_str(), join_ip.c_str()));
            containers.push_back({network_name, rpc_port, peer_port, join_ip});
          }
          manager.reset();
        }

        std::string insert_stmt = "";
        // store docker container and blockchain shard information in
        // trustdble.docker_containers table
        if (shared_database.bc_type.compare("ETHEREUM") == 0) {
          insert_stmt = " INSERT INTO trustdble.docker_containers ";
          insert_stmt.append(
              "(container_name, database_name, blockchain_type, rpc_port, "
              "peer_port, join_ip) VALUES ");
          for (auto &container : containers) {
            insert_stmt.append(
                "('" + container.container_name + "','" + shared_database.name +
                "','" + shared_database.bc_type + "','" + container.rpc_port +
                "','" + container.peer_port + "','" + container.join_ip + "')");
            if (&container != &containers.back()) {
              insert_stmt.append(", ");
            } else {
              insert_stmt.append(";");
            }
          }
          command_queries.append(insert_stmt);
        }

        // insert into data_chains table information about data-chains (shards)
        insert_stmt = " USE " + shared_database.name + ";" +
                      " INSERT INTO data_chains VALUES ";
        for (int shard_number = initial_num_shards; shard_number < num_shards;
             shard_number++) {
          insert_stmt.append(
              "(" + std::to_string(shard_number) + ",'" +
              // shifted value
              data_chains_network_config[shard_number - initial_num_shards] +
              "')");
          if (shard_number < num_shards - 1) {
            insert_stmt.append(", ");
          } else {
            insert_stmt.append(";");
          }
        }
        command_queries.append(insert_stmt);
      }
      // remove shards
      else if (delta_num_shards < 0) {
        // leave data chains
        // shard numeration starts from 0, for n-shard: n-1,n-2,..,1,0
        for (int shard_number = initial_num_shards - 1;
             shard_number > num_shards - 1; shard_number--) {
          DBUG_PRINT(
              LOG_TAG,
              ("partition_database: leave data chains, shard_number = %d",
               shard_number));
          std::unique_ptr<BcManagerNetworkConfig> data_network_config =
              std::make_unique<BcManagerNetworkConfig>();
          // std::unique_ptr<BcManagerNetworkConfig> data_network_config =
          // nullptr;

          // network name for data blockchain network
          std::string network_name = create_network_name(
              "data", database_name, config.config_server_number, shard_number);
          DBUG_PRINT(LOG_TAG, ("partition_database: define network name = %s",
                               network_name.c_str()));

          manager = BcManagerFactory::create_manager(
              BcManagerFactory::getBC_TYPE(shared_database.bc_type));
          if (manager == nullptr) {
            LogPluginErr(
                ERROR_LEVEL, ER_REWRITER_QUERY_FAILED,
                "ManagerFactory creation failed due to invalid bctype!");
            return 1;
          }

          // used when bctype is STUB or ETHEREUM
          manager->init(config.config_configuration_path);

          // get data_chains_network_config from data_chains table
          std::string data_chains_network_config_srt = "";
          GetDataNetworkConfigForShard(data_chains_network_config_srt,
                                       database_name, shard_number);
          DBUG_PRINT(LOG_TAG,
                     ("partition_database: data_chains_network_config_srt = %s",
                      data_chains_network_config_srt.c_str()));

          data_network_config->read(data_chains_network_config_srt);
          // check leave network is successful
          if (manager->leave_network(network_name, data_network_config) == 1) {
            LogPluginErr(ERROR_LEVEL, ER_REWRITER_QUERY_FAILED,
                         "DataChain node removal failed!");
            return 1;
          }
          manager.reset();
          DBUG_PRINT(LOG_TAG, ("partition_database: data chain %s is removed",
                               network_name.c_str()));
        }

        // delete information about data-chains (shards) from data_chains table
        std::string delete_stmt = "";
        delete_stmt = " DELETE FROM " + database_name +
                      ".data_chains WHERE datachain_id IN (";
        // for each removing shard_number
        for (int shard_number = initial_num_shards; shard_number > num_shards;
             shard_number--) {
          // data_chain_id numeration starts from 0
          delete_stmt.append(std::to_string(shard_number - 1));
          if (shard_number > num_shards + 1) {
            delete_stmt.append(",");
          } else {
            delete_stmt.append(");");
          }
        }
        DBUG_PRINT(LOG_TAG, ("partition_database: delete from data_chains = %s",
                             delete_stmt.c_str()));
        command_queries.append(delete_stmt);

        // delete docker containers information and blockchain shard information
        // in trustdble.docker_containers table
        if (shared_database.bc_type.compare("ETHEREUM") == 0) {
          delete_stmt =
              " DELETE FROM trustdble.docker_containers WHERE container_id IN "
              "(";
          // for each removing shard_number
          for (int shard_number = initial_num_shards; shard_number > num_shards;
               shard_number--) {
            // container numeration for data chain starts from 1
            delete_stmt.append(std::to_string(shard_number + 1));
            if (shard_number > num_shards + 1) {
              delete_stmt.append(",");
            } else {
              delete_stmt.append(");");
            }
          }
          DBUG_PRINT(LOG_TAG,
                     ("partition_database: delete from docker_containers = %s",
                      delete_stmt.c_str()));
          command_queries.append(delete_stmt);
        }

        DBUG_PRINT(LOG_TAG, ("partition_database: command_queries = %s",
                             command_queries.c_str()));
      }

      // don't run partition_tables command if no data tables in database
      if (!list_of_tables.empty()) {
        // remove shards
        if (delta_num_shards < 0) {
          // delete information about shards (partitions) from shared_tables
          // table
          std::string delete_stmt = "";
          // for each table in database
          for (auto table_name : list_of_tables) {
            DBUG_PRINT(LOG_TAG, ("partition_database: table_name = %s",
                                 table_name.c_str()));
            delete_stmt.append(" DELETE FROM " + database_name +
                               ".shared_tables WHERE partition_id IN (");
            // for each removing shard_number
            for (int shard_number = initial_num_shards;
                 shard_number > num_shards; shard_number--) {
              // get partition_id = table_name + '/' + data_chain_id
              std::string partition_id =
                  "'" + table_name + '/' + std::to_string(shard_number - 1) +
                  "'";  // partition_id numeration starts from 0
              delete_stmt.append(partition_id);
              if (shard_number > num_shards + 1) {
                delete_stmt.append(",");
              } else {
                delete_stmt.append(");");
              }
            }
          }
          DBUG_PRINT(LOG_TAG,
                     ("partition_database: delete from shared_tables = %s",
                      delete_stmt.c_str()));
          command_queries.append(delete_stmt);
        }

        // partition tables of database
        std::string partition_tables =
            " PARTITION TABLES " + database_name +
            " shards=" + std::to_string(initial_num_shards) + ";";
        DBUG_PRINT(LOG_TAG, ("partition_database: partition_tables = %s",
                             partition_tables.c_str()));
        command_queries.append(partition_tables);
      }
      break;
    }
    case TrustdbleSharedCommand::partition_tables: {
      DBUG_PRINT(LOG_TAG, ("trustdble_command_method_call: partition_tables"));
      command_queries = "";
      // get database name
      std::string database_name = parameter_Set["database_name"].result;
      // check database name is valid (not empty)
      if (database_name == "") {
        LogPluginErr(ERROR_LEVEL, ER_REWRITER_QUERY_FAILED,
                     "Invalid database name!");
        command_queries = "SIGNAL SQLSTATE '42S01' SET MYSQL_ERRNO='1050', ";
        command_queries.append(
            "MESSAGE_TEXT = 'partition_tables: Invalid database name!';");
        return 1;
      }
      DBUG_PRINT(LOG_TAG, ("partition_tables: database_name = %s",
                           database_name.c_str()));

      // get value of parameter initial number of shards (data-chains)
      PARSE_PARAMETER initial_numshards = parameter_Set["shards"];
      int initial_num_shards;
      try {
        initial_num_shards = boost::lexical_cast<int>(initial_numshards.result);
      } catch (boost::bad_lexical_cast const &e) {  // invalid input
        LogPluginErr(ERROR_LEVEL, ER_REWRITER_QUERY_FAILED,
                     "Invalid number of shards, can't parse parameter string!");
        return 1;
      }
      // check initial_num_shards is valid (>0)
      if (!(initial_num_shards > 0)) {
        LogPluginErr(
            ERROR_LEVEL, ER_REWRITER_QUERY_FAILED,
            "Invalid number of shards, number of shards should be > 0! %s",
            initial_numshards.result.c_str());
        return 1;
      }
      DBUG_PRINT(LOG_TAG, ("partition_tables: initial_num_shards = %d",
                           initial_num_shards));

      // get number of shards
      int num_shards = getNumShards(database_name);
      // check number of shards is valid (>0)
      if (!(num_shards > 0)) {
        LogPluginErr(
            ERROR_LEVEL, ER_REWRITER_QUERY_FAILED,
            "Invalid number of shards, number of shards should be > 0! %d",
            num_shards);
        command_queries = "SIGNAL  SQLSTATE '42S01' SET MYSQL_ERRNO='1050', ";
        command_queries.append(
            "MESSAGE_TEXT = 'partition_tables: Invalid number of shards, "
            "number of shards should be > 0!';");
        return 1;
      }
      DBUG_PRINT(LOG_TAG, ("partition_tables: num_shards = %d", num_shards));

      trustdble::SHARED_DATABASE shared_database;
      const char *shared_db;
      shared_db = database_name.c_str();

      // fetching the shared database
      if (trustdble::getSharedDatabase(shared_database, shared_db) == 0) {
        DBUG_PRINT(
            LOG_TAG,
            ("partition_tables: shared database: %s, %s, %s, %s",
             shared_database.name.c_str(), shared_database.bc_type.c_str(),
             shared_database.meta_address.c_str(),
             shared_database.meta_chain_config.c_str()));
      } else {
        LogPluginErr(ERROR_LEVEL, ER_REWRITER_QUERY_FAILED,
                     "partition_tables: database does not exist!");
        command_queries =
            "SIGNAL SQLSTATE '42S01' SET MYSQL_ERRNO='1050', MESSAGE_TEXT = "
            "'partition_tables: database does not exist!';";
        return 1;
      }

      // vector to store table_name information for all data tables (tables on
      // data chains)
      std::vector<std::string> list_of_tables;
      get_list_of_tables(list_of_tables, database_name.c_str());

      // for each table in database
      for (auto table : list_of_tables) {
        DBUG_PRINT(LOG_TAG,
                   ("partition_tables: table_name = %s", table.c_str()));
        // vector to store (PK) key for all rows in table
        std::vector<std::string> list_of_keys;

        // table data
        std::vector<std::vector<std::string>> table_data;
        // table scheme
        std::string table_schema = "";
        // get schema of table
        get_table_schema(table, table_schema, shared_db);
        DBUG_PRINT(LOG_TAG, ("partition_tables: table_schema = %s",
                             table_schema.c_str()));

        // copy table data from local table trustdble.tmp_table_name
        std::string local_table_name = "tmp_" + table;
        const char *local_db = "trustdble";
        copy_table(local_db, local_table_name.c_str(), table_data);
        for (const auto &row : table_data) {
          for (const auto &cell : row) {
            DBUG_PRINT(LOG_TAG,
                       ("partition_tables: copy_table: local_table_cell = %s",
                        cell.c_str()));
          }
        }

        // get table address for each data chain and update system table with
        // that data drop table
        std::string drop_table =
            " drop table " + database_name + "." + table + ";";
        command_queries.append(drop_table);
        // load table
        std::string load_table =
            " load shared table " + database_name + "." + table + ";";
        command_queries.append(load_table);

        // insert each row into table
        for (const auto &row : table_data) {
          std::string insert_row =
              " insert into " + database_name + "." + table + " values ";
          std::string table_row = "(";
          int col_num = 0;
          for (const auto &cell : row) {
            // get schema for the cell
            std::string field_schema =
                parse_table_schema(col_num, table_schema);
            DBUG_PRINT(LOG_TAG, ("partition_tables: field_schema = %s",
                                 field_schema.c_str()));
            // add single quotes for the cell, if type in NOT INT
            std::string formated_cell = format_query(cell, field_schema);
            DBUG_PRINT(LOG_TAG, ("partition_tables: formated_cell = %s",
                                 formated_cell.c_str()));
            table_row.append(formated_cell);
            if (&cell != &row.back()) {
              table_row.append(",");
            } else {
              table_row.append(")");
            }
            col_num++;  // next cell -> next table_column
          }
          insert_row.append(table_row);
          insert_row.append(";");
          DBUG_PRINT(LOG_TAG,
                     ("partition_tables: insert_row = %s", insert_row.c_str()));
          command_queries.append(insert_row);
        }

        // delete local tmp table
        std::string delete_local_table =
            " drop table trustdble." + local_table_name + ";";
        command_queries.append(delete_local_table);

        DBUG_PRINT(LOG_TAG, ("partition_tables: command_queries = %s",
                             command_queries.c_str()));
      }
      break;
    }
    case TrustdbleSharedCommand::drop_database: {
      DBUG_PRINT(LOG_TAG, ("trustdble_command_method_call: drop_database"));
      // shared database obj
      trustdble::SHARED_DATABASE shared_database;

      // rewrite query
      rewrite_query(command_queries, DROP_DATABASE);

      // extract parameter
      const char *db_name = parameter_Set["database_name"].result.c_str();

      // new_query = DROP_DATABASE.UNSHARED + parameters;

      // collection to store container_name
      std::vector<std::string> container_name;
      get_container_name(container_name, db_name);

      // fetching the shared database object
      if (trustdble::getSharedDatabase(shared_database, db_name) == 0) {
        DBUG_PRINT(
            LOG_TAG,
            ("Rewrite-Plugin: shared database: %s, %s, %s, %s, %s",
             shared_database.name.c_str(), shared_database.bc_type.c_str(),
             shared_database.meta_address.c_str(),
             shared_database.meta_chain_config.c_str(),
             shared_database.meta_chain_config.c_str()));
      } else {
        return 1;
      }

      // number of data-chains (shards)
      int num_shards = getNumShards(std::string(db_name));
      DBUG_PRINT(LOG_TAG,
                 ("Rewrite-Plugin: Get number of shards %d", num_shards));
      // string collection to store data_network_config information about
      // data-chains
      std::vector<std::string> data_chains_network_config;
      GetDataNetworkConfig(data_chains_network_config, db_name);
      DBUG_PRINT(LOG_TAG, ("Rewrite-Plugin: get_data_network_config %s",
                           data_chains_network_config[0].c_str()));

      // network name
      std::string network_name = "";

      // BC manager
      std::unique_ptr<BcManager> manager;

      // start DATA blockchain network (containers from 1 to size) - works only
      // for ETH
      // TODO update docker_container table for STUB + FABRIC
      for (int shard_number = 0; shard_number < num_shards; shard_number++) {
        std::unique_ptr<BcManagerNetworkConfig> data_network_config =
            std::make_unique<BcManagerNetworkConfig>();

        // name of container for data blockchain network
        network_name = create_network_name(
            "data", db_name, config.config_server_number, shard_number);
        // TODO update docker_container table for STUB + FABRIC
        // network_name = container_name[container_number];
        DBUG_PRINT(LOG_TAG, ("Rewrite-Plugin: Define network name %s",
                             network_name.c_str()));

        manager = BcManagerFactory::create_manager(
            BcManagerFactory::getBC_TYPE(shared_database.bc_type));
        if (manager == nullptr) {
          LogPluginErr(ERROR_LEVEL, ER_REWRITER_QUERY_FAILED,
                       "ManagerFactory creation failed due to invalid bctype!");
          return 1;
        }

        // used when bctype is STUB or ETHEREUM
        manager->init(config.config_configuration_path);

        data_network_config->read(data_chains_network_config[shard_number]);

        // check network creation is successful
        if (manager->leave_network(network_name, data_network_config) == 1) {
          LogPluginErr(ERROR_LEVEL, ER_REWRITER_QUERY_FAILED,
                       "DataChain node removal failed!");
          return 1;
        }
      }
      DBUG_PRINT(LOG_TAG,
                 ("Rewrite-Plugin: Data-chains are removed %s", db_name));

      // start META blockchain network (container 0)
      std::unique_ptr<BcManagerNetworkConfig> meta_network_config =
          std::make_unique<BcManagerNetworkConfig>();

      // get name of container for meta blockchain network
      network_name =
          create_network_name("meta", db_name, config.config_server_number);
      DBUG_PRINT(LOG_TAG, ("Rewrite-Plugin: Define network name %s",
                           network_name.c_str()));

      manager = BcManagerFactory::create_manager(
          BcManagerFactory::getBC_TYPE(shared_database.bc_type));
      if (manager == nullptr) {
        LogPluginErr(ERROR_LEVEL, ER_REWRITER_QUERY_FAILED,
                     "ManagerFactory creation failed due to invalid bctype!");
        return 1;
      }

      // used when bctype is STUB or ETHEREUM
      manager->init(config.config_configuration_path);
      // check network creation is successful

      std::string network_config = shared_database.meta_chain_config;
      meta_network_config->read(network_config);

      // check network creation is successful
      if (manager->leave_network(network_name, meta_network_config) == 1) {
        LogPluginErr(ERROR_LEVEL, ER_REWRITER_QUERY_FAILED,
                     "MetaChain node removal failed!");
        return 1;
      }
      DBUG_PRINT(LOG_TAG,
                 ("Rewrite-Plugin: Meta-chain is removed %s", db_name));

      command_queries.append(";");
      // add statements to query for deletion from database meta table
      std::string delete_stmt =
          "delete from trustdble.shared_databases where database_name = '" +
          shared_database.name + "';";
      command_queries.append(delete_stmt);

      // delete information about docker containers for dropped database
      delete_stmt =
          "delete from trustdble.docker_containers where database_name = '" +
          shared_database.name + "';";
      command_queries.append(delete_stmt);

      break;
    }
    case TrustdbleSharedCommand::load_database: {
      DBUG_PRINT(LOG_TAG, ("trustdble_command_method_call: load_database"));
      // shared database obj
      trustdble::SHARED_DATABASE invite;
      // extract parameter
      const char *db_name = parameter_Set["database_name"].result.c_str();
      // get db_name
      std::string db_name_str = parameter_Set["database_name"].result;

      if (getInvite(db_name, invite) == 1) {
        // read invite from invites table failed
        LogPluginErr(ERROR_LEVEL, ER_REWRITER_QUERY_FAILED,
                     "There is no invite for database");
        command_queries =
            "SIGNAL SQLSTATE 'HY000' SET MYSQL_ERRNO='1004', MESSAGE_TEXT = "
            "'There is no invite for database" +
            db_name_str + "';";
        return 1;
      }

      DBUG_PRINT(LOG_TAG,
                 ("Rewrite-Plugin:invites: %s, %s, %s, %s", invite.name.c_str(),
                  invite.bc_type.c_str(), invite.meta_address.c_str(),
                  invite.meta_chain_config.c_str()));

      // rewrite query
      command_queries = "create database " + db_name_str;

      // add change database to query
      command_queries.append("; USE " + db_name_str);

      // join meta blockchain network
      std::unique_ptr<BcManagerNetworkConfig> meta_network_config = nullptr;

      // name of container for META blockchain network
      std::string network_name = "";
      network_name.append(NETWORK_NAME_PREFIX_META);     // "metaBC"
      network_name.append(config.config_server_number);  // + server_number
      network_name.append(db_name);                      // + db_name

      std::unique_ptr<BcManager> manager = BcManagerFactory::create_manager(
          BcManagerFactory::getBC_TYPE(invite.bc_type));
      if (manager == nullptr) {
        LogPluginErr(ERROR_LEVEL, ER_REWRITER_QUERY_FAILED,
                     "ManagerFactory creation failed due to invalid bctype!");
        return 1;
      }

      manager->init(config.config_configuration_path);
      // get network_config for meta chain
      meta_network_config = std::make_unique<BcManagerNetworkConfig>();
      meta_network_config->read(invite.meta_chain_config);

      // join network
      if (manager->join_network(network_name, meta_network_config) == 1) {
        LogPluginErr(ERROR_LEVEL, ER_REWRITER_QUERY_FAILED,
                     "Joining MetaChain failed!");
        return 1;
      }

      invite.meta_chain_config = meta_network_config->get_json();
      manager.reset();

      // We have to wait for blockchain to be able to process transactions
      // TODO: whats going on on the blockchain when joining? Loads blocks?
      // Loads contracts?
      usleep(1000000 * 20);

      command_queries.append(";");

      // add statements to query for insert into database meta table
      std::stringstream meta_chain_config_ss;
      meta_chain_config_ss << quoted(invite.meta_chain_config);
      std::string meta_chain_config = meta_chain_config_ss.str();
      // remove all whitespace characters
      boost::replace_all(meta_chain_config, " ", "");

      // add statements to query for insert into database meta table
      std::string insert_stmt =
          "insert into trustdble.shared_databases (database_name, "
          "blockchain_type, meta_table_address, meta_chain_config, "
          "encryption_key, iv) values ('" +
          invite.name + "','" + invite.bc_type + "','" + invite.meta_address +
          "', " + meta_chain_config + ",'" + invite.encryption_key + "','" +
          invite.iv + "'" + ");";
      // insert meta_data about shared db into trustdble.shared_databases
      command_queries.append(insert_stmt);

      // create statement
      std::string create_stmt = " USE " + db_name_str + ";";
      // create meta-table
      create_stmt = " create table " + db_name_str + "." + META_TABLE_NAME +
                    " " + META_TABLE_SCHEMA + " " + ENGINE_STRING + ";";
      command_queries.append(create_stmt);

      // create shared table data_chains on meta-chain
      create_stmt = " create table " + db_name_str + "." +
                    META_TABLE_DATA_CHAINS_NAME + " " +
                    META_TABLE_DATA_CHAINS_SCHEMA + ENGINE_STRING + ";";
      command_queries.append(create_stmt);

      // create shared table shared_tables on meta-chain
      create_stmt = " create table " + db_name_str + "." + SHARED_TABLES_NAME +
                    " " + SHARED_TABLES_SCHEMA + ENGINE_STRING + ";";
      command_queries.append(create_stmt);

      // update meta-table
      create_stmt =
          " update " + META_TABLE_NAME + " set tableschema='" +
          SHARED_TABLES_SCHEMA.substr(1, SHARED_TABLES_SCHEMA.length() -
                                             2) +  // -2 to cut bracket symbol
          "' where tablename='" +
          SHARED_TABLES_NAME + "';";
      command_queries.append(create_stmt);

      // create shared key_store
      create_stmt = " create table " + db_name_str + "." +
                    KEY_STORE_TABLE_NAME + " " + KEY_STORE_TABLE_SCHEMA + ";";
      command_queries.append(create_stmt);

      // joint data_chain networks
      std::string load_shards = " LOAD SHARDS FROM " + db_name_str + ";";
      command_queries.append(load_shards);

      if (deleteInvite(db_name) == 1) {
        LogPluginErr(ERROR_LEVEL, ER_REWRITER_QUERY_FAILED,
            "could not delete invite for database");
        //command_queries = "SIGNAL SQLSTATE 'HY000' SET MYSQL_ERRNO='1004', MESSAGE_TEXT = 'There is no invite for database"+ db_name_str + "';";
        return 1;
      }



      // load shared tables
      // std::string load_tables = " load shared tables;";
      // command_queries.append(load_tables);

      break;
    }
    case TrustdbleSharedCommand::load_shards: {
      DBUG_PRINT(LOG_TAG, ("trustdble_command_method_call: load_shards"));

      command_queries = "";
      // get value of parameter database name
      std::string db_name = parameter_Set["database_name"].result;
      DBUG_PRINT(LOG_TAG, ("load_shards: db_name = %s", db_name.c_str()));
      // check db_name is valid
      if (db_name == "") {
        LogPluginErr(ERROR_LEVEL, ER_REWRITER_QUERY_FAILED,
                     "Invalid database name!");
        return 1;
      }

      trustdble::SHARED_DATABASE shared_database;
      // fetching the shared database
      if (getSharedDatabase(shared_database, db_name.c_str()) == 0) {
        DBUG_PRINT(LOG_TAG, ("Rewrite-Plugin: Invite: %s, %s, %s, %s",
                             shared_database.name.c_str(),
                             shared_database.bc_type.c_str(),
                             shared_database.meta_address.c_str(),
                             shared_database.meta_chain_config.c_str()));
      }
      // collection to store connection information
      std::vector<CONTAINER_INFO> containers;
      if (shared_database.bc_type.compare("ETHEREUM") == 0) {
        // network name
        std::string network_name = "";
        // name of container for META blockchain network
        network_name =
            create_network_name("meta", db_name, config.config_server_number);
        // parse connection information
        std::stringstream ss_data;
        ss_data << shared_database.meta_chain_config;
        pt::ptree net_config;
        pt::read_json(ss_data, net_config);
        // store connection information
        std::string rpc_port = net_config.get<std::string>("Network.rpc-port");
        std::string peer_port =
            net_config.get<std::string>("Network.peer-port");
        std::string join_ip = net_config.get<std::string>("Network.join-ip");
        containers.push_back({network_name, rpc_port, peer_port, join_ip});
      }

      // string collection to store data_network_config information about
      // data-chains
      std::vector<std::string> data_chains_network_config;
      GetDataNetworkConfig(data_chains_network_config, db_name.c_str());

      // number of data-chains (shards)
      int num_shards = getNumShards(db_name);
      DBUG_PRINT(LOG_TAG, ("load_shards: Get number of shards %d", num_shards));

      // create multiple data chains
      std::unique_ptr<BcManager> manager;
      for (int shard_number = 0; shard_number < num_shards; shard_number++) {
        std::unique_ptr<BcManagerNetworkConfig> data_network_config = nullptr;

        // name of container for data blockchain network
        std::string network_name = create_network_name(
            "data", db_name, config.config_server_number, shard_number);
        DBUG_PRINT(LOG_TAG, ("load_shards: Define network name %s",
                             network_name.c_str()));

        manager = BcManagerFactory::create_manager(
            BcManagerFactory::getBC_TYPE(shared_database.bc_type));
        if (manager != nullptr) {
          manager->init(config.config_configuration_path);
          // get network_config for data chain from table data_chains
          data_network_config = std::make_unique<BcManagerNetworkConfig>();
          DBUG_PRINT(LOG_TAG,
                     ("Rewrite-Plugin: Get shards %s",
                      data_chains_network_config[shard_number].c_str()));
          data_network_config->read(data_chains_network_config[shard_number]);

          // join network
          if (manager->join_network(network_name, data_network_config) == 1) {
            LogPluginErr(ERROR_LEVEL, ER_REWRITER_QUERY_FAILED,
                         "Joining DataChain failed!");
            command_queries =
            "SIGNAL SQLSTATE 'HY000' SET MYSQL_ERRNO='1003', MESSAGE_TEXT = "
            "'Joining data chain failed';";
            return 1;
          }
          if (shared_database.bc_type.compare("ETHEREUM") == 0) {
            // get connection information
            std::stringstream ss_data;
            ss_data << data_network_config->get_json();
            pt::ptree net_config;
            pt::read_json(ss_data, net_config);
            // store connection information
            std::string rpc_port =
                net_config.get<std::string>("Network.rpc-port");
            std::string peer_port =
                net_config.get<std::string>("Network.peer-port");
            std::string join_ip =
                net_config.get<std::string>("Network.join-ip");
            containers.push_back({network_name, rpc_port, peer_port, join_ip});
          }

          manager.reset();
        }
      }

      if (shared_database.bc_type.compare("ETHEREUM") == 0) {
        // store docker container and blockchain shard information in
        // trustdble.docker_containers table
        std::string insert_stmt = "INSERT INTO trustdble.docker_containers ";
        insert_stmt.append(
            "(container_name, database_name, blockchain_type, rpc_port, "
            "peer_port, join_ip) VALUES ");
        for (auto &container : containers) {
          insert_stmt.append(
              "('" + container.container_name + "','" + shared_database.name +
              "','" + shared_database.bc_type + "','" + container.rpc_port +
              "','" + container.peer_port + "','" + container.join_ip + "')");
          if (&container != &containers.back()) {
            insert_stmt.append(", ");
          } else {
            insert_stmt.append(";");
          }
        }
        command_queries.append(insert_stmt);
      } else {
        command_queries.append("show tables;");
      }

      // update database_state to CREATED state
      std::string update_stmt =
          " update trustdble.shared_databases "
          "set database_state='CREATED' "
          "where database_name = '" +
          db_name + "'" + ";";
      command_queries.append(update_stmt);

      break;
    }
    case TrustdbleSharedCommand::create_table: {
      DBUG_PRINT(LOG_TAG, ("trustdble_command_method_call: create_table"));
      rewrite_query(command_queries, CREATE_TABLE);
      command_queries.append(ENGINE_STRING);
      DBUG_PRINT(LOG_TAG, ("Rewrite-Plugin: create_table, command_queries %s",
                           command_queries.c_str()));
      std::string table_name = parameter_Set["table_name"].result;
      std::string table_schema = parameter_Set["schema"].result;
      // generating encryption key for each created table and insert into
      // meta_table
      std::string encryption_key;
      std::string iv;
      boost::algorithm::trim(table_name);
      table_schema = table_schema.substr(1, table_schema.length() - 2);
      boost::algorithm::trim(table_schema);
      std::string database_name;
      // Load shared database table
      trustdble::SHARED_DATABASE shared_database;

      const char *shared_db;
      if (table_name.find(".") != std::string::npos) {
        // parsing the parameter table_name to get the database name and the
        // table name
        database_name = table_name.substr(0, table_name.find("."));
        table_name =
            table_name.substr(table_name.find(".") + 1,
                              table_name.length() - table_name.find(".") - 1);
        // fetching a pointer to the database name
        shared_db = database_name.c_str();
      } else {
        // fetching a pointer to the database name from the current thread being
        // used
        shared_db = thd->db().str;
      }
      DBUG_PRINT(LOG_TAG,
                 ("Rewrite-Plugin: create_table, shared_db = %s", shared_db));

      if (trustdble::getSharedDatabase(shared_database, shared_db) == 0) {
        DBUG_PRINT(LOG_TAG, ("Rewrite-Plugin: Shared database: %s, %s, %s, %s",
                             shared_database.name.c_str(),
                             shared_database.bc_type.c_str(),
                             shared_database.meta_address.c_str(),
                             shared_database.meta_chain_config.c_str()));
        // if corresponding database has encryption key then create encryption
        // key created table and insert into meta_table
        if (!shared_database.encryption_key.empty()) {
          unsigned char *tbl_encryption_key = new unsigned char[KEY_SIZE];
          unsigned char *tbl_iv = new unsigned char[IV_SIZE];
          RAND_bytes(tbl_encryption_key, KEY_SIZE);
          RAND_bytes(tbl_iv, IV_SIZE);
          encryption_key = charArrayToHex(tbl_encryption_key, KEY_SIZE);
          iv = charArrayToHex(tbl_iv, IV_SIZE);
          delete[] tbl_encryption_key;
          delete[] tbl_iv;
        }
        // Read statement from meta-table
        std::string stmt;
        if (getStmt(table_name, stmt, shared_db) == 0) {
          // separate command_queries from update statement
          command_queries.append(";");

          // check if table is already in the shared db (on the data_chain_id=0)
          if (!checkTable(shared_db, table_name, 0)) {
            // number of data-chains (shards)
            int num_shards = getNumShards(shared_db);
            TableService tablereader;
            for (int shard_number = 0; shard_number < num_shards;
                 shard_number++) {
              // get partition_id = table_name + '/' + data_chain_id
              const std::string partition_id =
                  table_name + '/' + std::to_string(shard_number);
              // update table_schema for created table in table shared_tables
              std::string insert_shared_tables_stmt =
                  " INSERT INTO " + std::string(shared_db) + "." +
                  SHARED_TABLES_NAME + " values ('" + partition_id +
                  "', '', '" + table_schema + "')";
              DBUG_PRINT(LOG_TAG,
                         ("Rewrite-Plugin: create_table: insert_stmt = %s",
                          insert_shared_tables_stmt.c_str()));
              tablereader.query(insert_shared_tables_stmt);

              // if table has encryption key/iv insert it into key+store table
              if (!encryption_key.empty()) {
                std::string insert_stmt =
                    "INSERT INTO " + std::string(shared_db) + "." +
                    KEY_STORE_TABLE_NAME +
                    "(tablename, encryptionkey, iv) values ('" + table_name +
                    "','" + encryption_key + "','" + iv + "'" + ");";
                tablereader.query(insert_stmt);
              }
            }
            DBUG_PRINT(LOG_TAG, ("Rewrite-Plugin: Schema of table %s is: %s",
                                 table_name.c_str(), table_schema.c_str()));
          } else {
            LogPluginErr(ERROR_LEVEL, ER_REWRITER_QUERY_FAILED,
                         "Table already exists!");
            command_queries =
                "SIGNAL  SQLSTATE '42S01' SET MYSQL_ERRNO='1050', MESSAGE_TEXT "
                "= 'Table already exists!';";
            return 1;
          }
        }
      } else {
        LogPluginErr(ERROR_LEVEL, ER_REWRITER_QUERY_FAILED,
                     "Reading database table failed!");
        return 1;
      }

      break;
    }
    case TrustdbleSharedCommand::drop_table: {
      DBUG_PRINT(LOG_TAG, ("trustdble_command_method_call: drop_table"));
      std::string table_name = parameter_Set["table_name"].result;
      boost::algorithm::trim(table_name);

      // get shared database name
      std::string database_name;
      const char *shared_db;
      if (table_name.find(".") != std::string::npos) {
        // parsing the parameter table_name to get the database name and the
        // table name
        database_name = table_name.substr(0, table_name.find("."));
        table_name =
            table_name.substr(table_name.find(".") + 1,
                              table_name.length() - table_name.find(".") - 1);
        // fetching a pointer to the database name
        shared_db = database_name.c_str();
      } else {
        // fetching a pointer to the database name from the current thread being
        // used
        shared_db = thd->db().str;
        database_name = std::string(shared_db);
      }

      command_queries = "drop table " + database_name + "." + table_name + ";";

      break;
    }
    case TrustdbleSharedCommand::load_table: {
      DBUG_PRINT(LOG_TAG, ("trustdble_command_method_call: load_table"));
      // look into meta-bc and retrieve create statement
      // key is the table name
      std::string table_name = parameter_Set["table_name"].result;
      ;
      boost::algorithm::trim(table_name);
      std::string stmt;

      std::string db_name;
      const char *shared_db;
      if (table_name.find(".") != std::string::npos) {
        // parsing the parameter table_name to get the database name and the
        // table name
        db_name = table_name.substr(0, table_name.find("."));
        table_name =
            table_name.substr(table_name.find(".") + 1,
                              table_name.length() - table_name.find(".") - 1);
        // fetching a pointer to the database name
        shared_db = db_name.c_str();
      } else {
        // fetching a pointer to the database name from the current thread being
        // used
        shared_db = thd->db().str;
      }
      // load shared database table
      trustdble::SHARED_DATABASE shared_database;

      if (trustdble::getSharedDatabase(shared_database, shared_db) == 0) {
        DBUG_PRINT(LOG_TAG, ("load_table: command_queries = %s",
                             command_queries.c_str()));
        // if query contains multiple statements, cut load_table and save the
        // tail of query in command_queries_tail
        std::string command_queries_tail = "";
        // check if query contains multiple statements (<;> as a separator)
        if (command_queries.find(';') != std::string::npos) {
          command_queries_tail = command_queries.substr(
              command_queries.find_first_of(';') + 1, command_queries.length());
          command_queries_tail.append(";");
          DBUG_PRINT(LOG_TAG, ("load_table: command_queries_tail = %s",
                               command_queries_tail.c_str()));
        }
        DBUG_PRINT(LOG_TAG, ("Rewrite-Plugin: Shared database: %s, %s, %s, %s",
                             shared_database.name.c_str(),
                             shared_database.bc_type.c_str(),
                             shared_database.meta_address.c_str(),
                             shared_database.meta_chain_config.c_str()));
        command_queries = " USE " + std::string(shared_db) + ";";
        if (!shared_database.encryption_key.empty()) {
          // check if the table-name exists in key_store if not show error and
          // exit
          trustdble::KEY_STORE table;
          if (getKeyStore(shared_db, table_name.c_str(), table) == 1) {
            LogPluginErr(ERROR_LEVEL, ER_REWRITER_QUERY_FAILED,
                         "Table doesn't exist in key store!");
            command_queries =
                "SIGNAL  SQLSTATE 'HY000' SET MYSQL_ERRNO='1032', MESSAGE_TEXT "
                "= 'Table does not exist in key store!';";
            return 1;
          }
        }
        // read statement from shared_tables
        if (getDataTable(table_name, stmt, shared_db) == 0) {
          // if (getStmt(thd, table_name, stmt, shared_db)==0) {
          if (stmt == "") {
            LogPluginErr(ERROR_LEVEL, ER_REWRITER_QUERY_FAILED,
                         "Table doesn't exist!");
            command_queries =
                "SIGNAL  SQLSTATE '42S02' SET MYSQL_ERRNO='1146', MESSAGE_TEXT "
                "= 'Table does not exist!';";
            return 1;
          } else {
            command_queries.append(stmt);
            command_queries.append(command_queries_tail);
            // command_queries.append(" show tables;");
            DBUG_PRINT(LOG_TAG, ("load_table: command_queries = %s",
                                 command_queries.c_str()));
          }
        } else {
          LogPluginErr(ERROR_LEVEL, ER_REWRITER_QUERY_FAILED,
                       "Reading database table failed!");
          return 1;
        }
      }
      break;
    }
    case TrustdbleSharedCommand::load_tables: {
      DBUG_PRINT(LOG_TAG, ("trustdble_command_method_call: load_tables"));

      // load shared database table
      trustdble::SHARED_DATABASE shared_database;
      // fetching a pointer to the database name from the current thread being
      // used
      const char *shared_db = thd->db().str;
      // get shares db meta_data
      if (trustdble::getSharedDatabase(shared_database, shared_db) == 0) {
        DBUG_PRINT(LOG_TAG, ("load_tables: shared database: %s, %s, %s, %s",
                             shared_database.name.c_str(),
                             shared_database.bc_type.c_str(),
                             shared_database.meta_address.c_str(),
                             shared_database.meta_chain_config.c_str()));

        // read all statements from table shared_tables
        std::vector<std::tuple<std::string, std::string>> meta_data;
        getMetaDataForSharedDataTable(meta_data, shared_db);
        // query string
        std::string stmt;
        // collection to store table_name for all data_tables
        std::vector<std::string> table_name_list;

        for (auto row : meta_data) {
          // partition_id = table_name + '/' + data_chain_id
          std::string partition_id = std::get<0>(row);
          // get table_name from partition_id
          std::string table_name =
              partition_id.substr(0, partition_id.find_last_of('/'));
          // check if table_name is NOT in table_name_list
          if (!(std::find(table_name_list.begin(), table_name_list.end(),
                          table_name) != table_name_list.end())) {
            // check if table_name exists in key_store if yes add the table name
            // to table_name_list
            trustdble::KEY_STORE key_store;
            if (getKeyStore(shared_db, table_name.c_str(), key_store) == 0) {
              // add table_name in table_name_list
              table_name_list.push_back(table_name);
              // get table_schema
              getDataTable(table_name, stmt, shared_db);
              // add create statement once for each table_name
              command_queries.append(stmt + " ");
            } else {
              DBUG_PRINT(LOG_TAG,
                         ("Rewrite-Plugin: Table doesn't exist in key store %s",
                          table_name.c_str()));
            }
          }
        }
        if (table_name_list.empty()) {
          LogPluginErr(ERROR_LEVEL, ER_REWRITER_QUERY_FAILED,
                       "Table doesn't exist in key store!");
          command_queries =
              "SIGNAL  SQLSTATE 'HY000' SET MYSQL_ERRNO='1032', MESSAGE_TEXT = "
              "'Table does not exist in key store!';";
          return 1;
        }
      } else {
        LogPluginErr(ERROR_LEVEL, ER_REWRITER_QUERY_FAILED,
                     "Reading database table failed!");
        return 1;
      }

      // remove shared tables string from query
      boost::replace_all(command_queries, "load shared tables", " ");
      boost::replace_all(command_queries, "LOAD SHARED TABLES", " ");
      // show all tables for the current db
      command_queries.append(" show tables;");
      break;
    }
    case TrustdbleSharedCommand::create_invite: {
      DBUG_PRINT(LOG_TAG, ("trustdble_command_method_call: create_invite"));
      // shared database obj
      trustdble::SHARED_DATABASE shared_database;
      // extract invite string and public key
      std::string database_name = parameter_Set["database_name"].result;
      // fetching the shared database info
      if (trustdble::getSharedDatabase(shared_database,
                                       database_name.c_str()) == 1) {
        DBUG_PRINT(LOG_TAG,
                   ("database does not exist %s", database_name.c_str()));
        command_queries =
            "SIGNAL SQLSTATE '42000' SET MYSQL_ERRNO='1049', MESSAGE_TEXT = "
            "'Unknown database " +
            database_name + "';";
        return 1;
      }
      std::string invite_string =
          "{\"database_name\":\"" + shared_database.name +
          "\",\"blockchain_type\":\"" + shared_database.bc_type +
          "\",\"meta_table_address\":\"" + shared_database.meta_address +
          "\",\"meta_chain_config\":" + shared_database.meta_chain_config +
          ",\"encryption_key\":\"" + shared_database.encryption_key +
          "\",\"iv\":\"" + shared_database.iv + "\"}";
      const unsigned char *invite_byte =
          (const unsigned char *)invite_string.c_str();

      std::string public_key_primary = parameter_Set["public_key"].result;
      std::string start_public = "-----BEGIN PUBLIC KEY-----\n";
      std::string end_public = "\n-----END PUBLIC KEY-----\n";
      std::string public_key =
          start_public.append(public_key_primary).append(end_public);
      const char *public_key_byte = public_key.c_str();
      int size = public_key_primary.length();
      if (size == 0) {
      }

      if (!is_base64_encoded(public_key_primary)) {
        DBUG_PRINT(LOG_TAG, ("public key is not valid"));
        command_queries =
            "SIGNAL SQLSTATE 'HY000' SET MYSQL_ERRNO='1003', MESSAGE_TEXT = "
            "'Public key is not valid';";
        return 1;
      }
      if (!validate_publickey(public_key_byte)) {
        ERR_clear_error();
        DBUG_PRINT(LOG_TAG, ("public key is not valid"));
        command_queries =
            "SIGNAL SQLSTATE 'HY000' SET MYSQL_ERRNO='1003', MESSAGE_TEXT = "
            "'Public key is not valid';";
        return 1;
      }
      command_queries = "";
      if (!invite_string.empty() && !public_key_primary.empty()) {
        // the maximum size of data that can be encrypted using public key is
        // the size of public key as most of invite strings size is more than
        // 512 bytes so:
        // 1. encrypt invite string using symmetric key
        // 2. encrypt symmetric key and iv using public key
        size_t invite_len = invite_string.length();
        // generate symmetric key and iv
        unsigned char *encryption_key = new unsigned char[KEY_SIZE];
        unsigned char *iv = new unsigned char[IV_SIZE];
        RAND_bytes(encryption_key, KEY_SIZE);
        RAND_bytes(iv, IV_SIZE);
        // encrypt invite string using symmetric encryption
        unsigned char *encrypted_invite = new unsigned char[invite_len + 256];
        size_t encrypted_invite_len = encrypt(
            invite_byte, invite_len, encryption_key, iv, encrypted_invite);
        // Error handling when encryption fails
        if (encrypted_invite_len == 0) {
          command_queries =
              "SIGNAL SQLSTATE 'HY000' SET MYSQL_ERRNO='1006', MESSAGE_TEXT = "
              "'Encryption failed';";
          delete[] encrypted_invite;
          delete[] encryption_key;
          delete[] iv;
          return 1;
        }
        // encrypt symmetric key and iv using public key
        BYTES encrypted_key, encrypted_iv;
        encrypted_key =
            encrypt_public(encryption_key, KEY_SIZE, public_key_byte);
        encrypted_iv = encrypt_public(iv, IV_SIZE, public_key_byte);
        // convert encrypted symmetric key/iv and encrypted invite into hex
        std::string encrypted_invite_str =
            byte_array_to_hex(encrypted_invite, encrypted_invite_len);
        std::string encrypted_key_str =
            byte_array_to_hex(encrypted_key.value, encrypted_key.size);
        std::string encrypted_iv_str =
            byte_array_to_hex(encrypted_iv.value, encrypted_iv.size);

        // create table encrypted_invite
        std::string create_stmt = " CREATE TABLE IF NOT EXISTS trustdble." +
                                  ENCRYPTED_INVITE_NAME + " " +
                                  ENCRYPTED_INVITE_SCHEMA + ";";

        TableService tablereader;
        int error = tablereader.query(create_stmt);

        if (error != 1) {
          // insert encrypted invite string into encrypted_invite table
          std::string insert_stmt = "INSERT INTO trustdble." + ENCRYPTED_INVITE_NAME + " VALUES ('" +
            public_key_primary + "','" + database_name + "','" + encrypted_key_str + "','" +
            encrypted_iv_str + "','" + encrypted_invite_str + "'" + ") ON DUPLICATE KEY UPDATE encrypted_key = '" +
            encrypted_key_str + "', encrypted_iv = '" + encrypted_iv_str + "', encrypted_invite = '" +
            encrypted_invite_str + "';";
          error = tablereader.query(insert_stmt);
        }

        if(error == 1) {
          command_queries = "select 'Create invite for database not successfully' as '';";
          LogPluginErr(ERROR_LEVEL, ER_REWRITER_QUERY_FAILED,
            "could not create invite for database");
          delete[] encrypted_invite;
          delete[] encryption_key;
          delete[] iv;
          return 1;
        }
        else {
          command_queries = "select 'Successfully create the invitation for the database' as '';";
          DBUG_PRINT(LOG_TAG, ("Successfully create the invitation for the database"));
        }

        DBUG_PRINT(LOG_TAG,
                   ("Rewrite-Plugin: %s", encrypted_invite_str.c_str()));

        delete[] encrypted_invite;
        delete[] encryption_key;
        delete[] iv;
      } else {
        DBUG_PRINT(
            LOG_TAG,
            ("Rewrite-Plugin: Read Invite failed OR Public key is empty"));
        return 1;
      }
      break;
    }
    case TrustdbleSharedCommand::add_invite: {
      DBUG_PRINT(LOG_TAG, ("trustdble_command_method_call: add_invite"));

      // encrypted invite obj
      trustdble::ENCRYPTED_STRUCT encrypted_invite;
      // extract encrypted invite in json format {"key":"","iv":"","invite":""}
      std::string encrypted_invite_string =
          parameter_Set["encrypted_invite"].result;

      if (encrypted_invite_string != "") {
        DBUG_PRINT(LOG_TAG, ("Rewrite-Plugin: Read encrtpted Invite from "
                             "encrypted invite string: %s",
                             encrypted_invite_string.c_str()));
        // read encrypted invite from invite string
        if (!parse_encrypted_invite(encrypted_invite_string.c_str(),
                                    encrypted_invite)) {
          LogPluginErr(ERROR_LEVEL, ER_REWRITER_QUERY_FAILED,
                       "Reading encrypted invite string failed!");
          command_queries =
              "SIGNAL SQLSTATE 'HY000' SET MYSQL_ERRNO='1001', MESSAGE_TEXT = "
              "'Encrypted invite file is not in the right format';";
          return 1;
        }
      } else {
        DBUG_PRINT(LOG_TAG, ("Rewrite-Plugin: Encrypted Invite is empty"));
        command_queries =
            "SIGNAL SQLSTATE 'HY000' SET MYSQL_ERRNO='1002', MESSAGE_TEXT = "
            "'Encrypted invite is empty';";
        return 1;
      }

      DBUG_PRINT(LOG_TAG, ("Rewrite-Plugin: %s, %s, %s",
                           encrypted_invite.encrypted_key.c_str(),
                           encrypted_invite.encrytpted_iv.c_str(),
                           encrypted_invite.encrypted_string.c_str()));

      std::string encrypted_key = encrypted_invite.encrypted_key;
      std::string encrypted_iv = encrypted_invite.encrytpted_iv;
      std::string encrypted_invite_str = encrypted_invite.encrypted_string;
      unsigned char *encrypted_key_byte =
          new unsigned char[encrypted_key.length() / 2];
      unsigned char *encrypted_iv_byte =
          new unsigned char[encrypted_iv.length() / 2];
      unsigned char *encrypted_invite_byte =
          new unsigned char[encrypted_invite_str.length() / 2];
      unsigned char *decrypted_invite_byte =
          new unsigned char[encrypted_invite_str.length() / 2];
      unsigned char *decrypted_key = new unsigned char[KEY_SIZE];
      unsigned char *decrypted_iv = new unsigned char[IV_SIZE];
      int key_error = hexToCharArray(encrypted_key, encrypted_key_byte);
      int iv_error = hexToCharArray(encrypted_iv, encrypted_iv_byte);
      int invite_error =
          hexToCharArray(encrypted_invite_str, encrypted_invite_byte);
      if (key_error == 1 || iv_error == 1 || invite_error == 1) {
        command_queries =
            "SIGNAL SQLSTATE 'HY000' SET MYSQL_ERRNO='1007', MESSAGE_TEXT = "
            "'Encrypted invite file is not in the right format';";
        delete[] encrypted_key_byte;
        delete[] encrypted_iv_byte;
        delete[] encrypted_invite_byte;
        delete[] decrypted_invite_byte;
        delete[] decrypted_key;
        delete[] decrypted_iv;
        return 1;
      }
      // decrypt encrypted key/iv using private key
      int decrypted_key_size =
          decrypt_private(encrypted_key_byte, encrypted_key.length() / 2,
                          config.config_configuration_path,
                          config.config_server_number, decrypted_key);
      int decrypted_iv_size =
          decrypt_private(encrypted_iv_byte, encrypted_iv.length() / 2,
                          config.config_configuration_path,
                          config.config_server_number, decrypted_iv);

      if (decrypted_key_size == -1 || decrypted_iv_size == -1) {
        ERR_clear_error();
        delete[] encrypted_key_byte;
        delete[] encrypted_iv_byte;
        delete[] encrypted_invite_byte;
        delete[] decrypted_invite_byte;
        delete[] decrypted_key;
        delete[] decrypted_iv;
        command_queries =
            "SIGNAL SQLSTATE 'HY000' SET MYSQL_ERRNO='1003', MESSAGE_TEXT = "
            "'Decrypting using private key failed';";
        LogPluginErr(ERROR_LEVEL, ER_REWRITER_QUERY_FAILED,
                     "Decrypting using private key failed!");
        return 1;
      }
      // decrypt encrypted invite using key/iv
      size_t decrypted_invite_len =
          decrypt(encrypted_invite_byte, encrypted_invite_str.length() / 2,
                  decrypted_key, decrypted_iv, decrypted_invite_byte);
      // Error handling when decryption fails
      if (decrypted_invite_len == 0) {
        command_queries =
            "SIGNAL SQLSTATE 'HY000' SET MYSQL_ERRNO='1006', MESSAGE_TEXT = "
            "'Encryption failed';";
        delete[] encrypted_key_byte;
        delete[] encrypted_iv_byte;
        delete[] encrypted_invite_byte;
        delete[] decrypted_invite_byte;
        delete[] decrypted_key;
        delete[] decrypted_iv;
        return 1;
      }
      decrypted_invite_byte[decrypted_invite_len] = '\0';
      std::string decrypted_invite_str(
          reinterpret_cast<char *>(decrypted_invite_byte));
      // shared database obj
      trustdble::SHARED_DATABASE shared_database;
      // read invite from invite string
      if (!parse_invite(decrypted_invite_str.c_str(), shared_database)) {
        delete[] encrypted_key_byte;
        delete[] encrypted_iv_byte;
        delete[] encrypted_invite_byte;
        delete[] decrypted_invite_byte;
        delete[] decrypted_key;
        delete[] decrypted_iv;
        LogPluginErr(ERROR_LEVEL, ER_REWRITER_QUERY_FAILED,
                     "Reading invite string failed!");
        return 1;
      }
      // add statements to query for insert into invites table
      std::stringstream meta_chain_config_ss;
      meta_chain_config_ss << quoted(shared_database.meta_chain_config);

      // create invites table
      std::string create_stmt = "CREATE TABLE IF NOT EXISTS trustdble." +
                                INVITES_NAME + " " + INVITES_SCHEMA + ";";
      TableService tablereader;
      int error = tablereader.query(create_stmt);

      if (error != 1) {
        // insert values into invites table
        std::string insert_stmt = "INSERT INTO trustdble." + INVITES_NAME +
          " (database_name, blockchain_type, meta_table_address, meta_chain_config, encryption_key, iv) VALUES('" +
          shared_database.name + "','" + shared_database.bc_type + "','" + shared_database.meta_address + "', " +
          meta_chain_config_ss.str() + " ,'" + shared_database.encryption_key + "','" + shared_database.iv + "'" + ");";
        error = tablereader.query(insert_stmt);
      }

      if(error == 1) {
        command_queries = "select 'Add invite for the database failed' as '';";
        LogPluginErr(ERROR_LEVEL, ER_REWRITER_QUERY_FAILED,
            "could not add invite for the database");
        delete[] encrypted_key_byte;
        delete[] encrypted_iv_byte;
        delete[] encrypted_invite_byte;
        delete[] decrypted_invite_byte;
        delete[] decrypted_key;
        delete[] decrypted_iv;
        return 1;
      } else {
        command_queries = "select 'Successfully add the invitation for the database' as '';";
        DBUG_PRINT(LOG_TAG, ("Successfully add the invitation for the database"));
      }

      delete[] encrypted_key_byte;
      delete[] encrypted_iv_byte;
      delete[] encrypted_invite_byte;
      delete[] decrypted_invite_byte;
      delete[] decrypted_key;
      delete[] decrypted_iv;
      break;
    }
    case TrustdbleSharedCommand::create_table_invite: {
      DBUG_PRINT(LOG_TAG,
                 ("trustdble_command_method_call: create_table_invite"));

      // get table name
      std::string table_name = parameter_Set["table_name"].result;
      boost::algorithm::trim(table_name);

      // get database name
      std::string db_name;
      const char *shared_db;
      if (table_name.find(".") != std::string::npos) {
        // parsing the parameter table_name to get the database name and the
        // table name
        db_name = table_name.substr(0, table_name.find("."));
        table_name =
            table_name.substr(table_name.find(".") + 1,
                              table_name.length() - table_name.find(".") - 1);
        // fetching a pointer to the database name
        shared_db = db_name.c_str();
      } else {
        // fetching a pointer to the database name from the current thread being
        // used
        shared_db = thd->db().str;
        db_name = std::string(shared_db);
      }

      // shared databse obj
      trustdble::SHARED_DATABASE shared_database;
      if (trustdble::getSharedDatabase(shared_database, shared_db) == 0) {
        DBUG_PRINT(LOG_TAG, ("Rewrite-Plugin: Shared database: %s",
                             shared_database.name.c_str()));
      }

      // get public_key
      std::string public_key_primary = parameter_Set["public_key"].result;

      // vector to store table_name information for all LOCAL tables in db_name
      std::vector<std::string> list_of_tables;
      get_list_of_local_tables(list_of_tables, db_name.c_str());

      command_queries = "";
      std::string invite_string = "";

      // if table is LOCAL table
      if (std::find(list_of_tables.begin(), list_of_tables.end(), table_name) !=
          list_of_tables.end()) {
        // get user name as a F(public_key)
        std::size_t user_name = std::hash<std::string>{}(public_key_primary+db_name+table_name);
        // get password
        // allocate new memory
        unsigned char *password_char =
            new unsigned char[KEY_SIZE];  // lenght of KEY_SIZE
        RAND_bytes(password_char, KEY_SIZE);
        std::string password = charArrayToHex(password_char, KEY_SIZE);
        DBUG_PRINT(LOG_TAG,
                   ("create_table_invite: password = %s", password.c_str()));
        // free memory
        delete[] password_char;

        // create user
        std::string create_smth = " CREATE USER IF NOT EXISTS '" +
                                  std::to_string(user_name) +
                                  "'@'%' IDENTIFIED BY '" + password + "';";
        TableService tablereader;
        int error = tablereader.query(create_smth);

        if (error != 1) {
          // create grant privileges
          std::string grant_smth = " GRANT SELECT ON " + db_name + "." + table_name +
                                  " TO '" + std::to_string(user_name) + "'@'%';";
          error = tablereader.query(grant_smth);
        }

        if (error == 1) {
          command_queries = "select 'Create user or grant privilege failed' as '';";
          LogPluginErr(ERROR_LEVEL, ER_REWRITER_QUERY_FAILED,
              "could not create user or grant privilege");
          return 1;
        } else {
          command_queries = "select 'Successfully create user and grant privilege' as '';";
          DBUG_PRINT(LOG_TAG, ("Successfully create user and grant privilege"));
        }

        // get table schema
        std::string table_schema = "";
        get_local_table_schema(table_schema, db_name, table_name);

        // get hostname
        std::string hostname = "";
        get_hostname(hostname);
        // get port
        std::string port = "";
        get_port(port);

        invite_string =
            "{\"db_name\":\"" + db_name + "\",\"table_name\":\"" + table_name +
            "\",\"table_schema\":\"" + table_schema + "\",\"hostname\":\"" +
            hostname + "\",\"port\":\"" + port + "\",\"user_name\":\"" +
            std::to_string(user_name) + "\",\"password\":\"" + password + "\"}";

        DBUG_PRINT(LOG_TAG, ("create_table_invite: invite_string = %s",
                             invite_string.c_str()));
      }
      // if table is SHARED table
      else {
        // fetching the key store info
        // key store obj
        trustdble::KEY_STORE key_store;
        if (trustdble::getKeyStore(shared_db, table_name.c_str(), key_store) ==
            1) {
          DBUG_PRINT(LOG_TAG, ("Unkown shared table  %s", table_name.c_str()));
          command_queries =
              "SIGNAL SQLSTATE '42S02' SET MYSQL_ERRNO='1109', MESSAGE_TEXT = "
              "'Unknown table " +
              table_name + "';";
          return 1;
        }

        invite_string = "{\"db_name\":\"" + shared_database.name +
                        "\",\"table_name\":\"" + key_store.table_name +
                        "\",\"encryption_key\":\"" + key_store.encryption_key +
                        "\",\"iv\":\"" + key_store.iv + "\"}";
      }

      // encrypt invite_string with the given public_key
      std::string start_public = "-----BEGIN PUBLIC KEY-----\n";
      std::string end_public = "\n-----END PUBLIC KEY-----\n";
      std::string public_key =
          start_public.append(public_key_primary).append(end_public);
      const char *public_key_byte = public_key.c_str();

      if (!is_base64_encoded(public_key_primary)) {
        DBUG_PRINT(LOG_TAG, ("public key is not valid"));
        command_queries =
            "SIGNAL SQLSTATE 'HY000' SET MYSQL_ERRNO='1003', MESSAGE_TEXT = "
            "'Public key is not valid';";
        return 1;
      }
      if (!validate_publickey(public_key_byte)) {
        ERR_clear_error();
        DBUG_PRINT(LOG_TAG, ("public key is not valid"));
        command_queries =
            "SIGNAL SQLSTATE 'HY000' SET MYSQL_ERRNO='1003', MESSAGE_TEXT = "
            "'Public key is not valid';";
        return 1;
      }
      if (!invite_string.empty() && !public_key_primary.empty()) {
        // encrypt invite string with public key
        size_t invite_len = invite_string.length();
        BYTES encrypted_invite =
            encrypt_public((const unsigned char *)invite_string.c_str(),
                           invite_len, public_key_byte);
        // convert encrypted invite into hex
        std::string encrypted_invite_str =
            byte_array_to_hex(encrypted_invite.value, encrypted_invite.size);

        // create table encrypted_table_invite
        std::string create_stmt = " CREATE TABLE IF NOT EXISTS trustdble." + ENCRYPTED_TABLE_INVITE_NAME + " " +
                    ENCRYPTED_TABLE_INVITE_SCHEMA  + ";";

        TableService tablereader;
        int error = tablereader.query(create_stmt);

        if (error != 1) {
          // truncate encrypted_table_invite table
          std::string truncate_stmt = "TRUNCATE TABLE trustdble." + ENCRYPTED_TABLE_INVITE_NAME +";";
          error = tablereader.query(truncate_stmt);
        }

        if (error != 1) {
          // insert encrypted invite string into encrypted_table_invite table
          std::string insert_stmt = "INSERT INTO trustdble."+ ENCRYPTED_TABLE_INVITE_NAME+" VALUES ('" +
            db_name + "." + table_name + "','" + encrypted_invite_str  +  "'" + ")"
            + "  ON DUPLICATE KEY UPDATE encrypted_invite = '"  +  encrypted_invite_str + "';";
          error = tablereader.query(insert_stmt);
        }

        if(error == 1) {
          command_queries = "select 'Create invite for table not successfully' as '';";
          LogPluginErr(ERROR_LEVEL, ER_REWRITER_QUERY_FAILED,
            "could not create invite for table");
          return 1;
        }
        else {
          command_queries = "select 'Successfully create the invitation for the table' as '';";
          DBUG_PRINT(LOG_TAG, ("Successfully create the invitation for the table"));
        }

        DBUG_PRINT(LOG_TAG,
                   ("Rewrite-Plugin: %s", encrypted_invite_str.c_str()));
      } else {
        DBUG_PRINT(
            LOG_TAG,
            ("Rewrite-Plugin: Read Invite failed OR Public key is empty"));
        return 1;
      }
      break;
    }
    case TrustdbleSharedCommand::add_table_invite: {
      DBUG_PRINT(LOG_TAG, ("trustdble_command_method_call: add_table_invite"));
      // fetching a pointer to the database name from the current thread being
      // used
      const char *shared_db = thd->db().str;
      if (shared_db == nullptr) {
        command_queries =
            "SIGNAL SQLSTATE '3D000' SET MYSQL_ERRNO='1046', MESSAGE_TEXT = "
            "'No database selected';";
        LogPluginErr(ERROR_LEVEL, ER_REWRITER_QUERY_FAILED,
                     "No database selected !");
        return 1;
      }

      std::string encrypted_invite;
      // extract encrypted invite in json format {"invite":""}
      std::string encrypted_invite_string =
          parameter_Set["encrypted_invite"].result;

      if (encrypted_invite_string != "") {
        DBUG_PRINT(LOG_TAG, ("Rewrite-Plugin: Read encrypted Invite from "
                             "encrypted invite string: %s",
                             encrypted_invite_string.c_str()));
        // read encrypted invite from invite string
        if (!parse_table_encrypted_invite(encrypted_invite_string.c_str(),
                                          encrypted_invite)) {
          LogPluginErr(ERROR_LEVEL, ER_REWRITER_QUERY_FAILED,
                       "Reading encrypted invite string failed!");
          command_queries =
              "SIGNAL SQLSTATE 'HY000' SET MYSQL_ERRNO='1001', MESSAGE_TEXT = "
              "'Encrypted invite file is not in the right format';";
          return 1;
        }
      } else {
        DBUG_PRINT(LOG_TAG, ("Rewrite-Plugin: Encrypted Invite is empty"));
        command_queries =
            "SIGNAL SQLSTATE 'HY000' SET MYSQL_ERRNO='1002', MESSAGE_TEXT = "
            "'Encrypted invite is empty';";
        return 1;
      }

      DBUG_PRINT(LOG_TAG, ("add_table_invite: encrypted_invite = %s",
                           encrypted_invite.c_str()));

      unsigned char *encrypted_invite_byte =
          new unsigned char[encrypted_invite.length() / 2];
      unsigned char *decrypted_invite_byte =
          new unsigned char[encrypted_invite.length() / 2];
      int error = hexToCharArray(encrypted_invite, encrypted_invite_byte);
      if (error == 1) {
        command_queries =
            "SIGNAL SQLSTATE 'HY000' SET MYSQL_ERRNO='1007', MESSAGE_TEXT = "
            "'Encrypted invite file is not in the right format';";
        delete[] encrypted_invite_byte;
        delete[] decrypted_invite_byte;
        return 1;
      }

      // decrypt encrypted invite using private key
      int decrypted_invite_size =
          decrypt_private(encrypted_invite_byte, encrypted_invite.length() / 2,
                          config.config_configuration_path,
                          config.config_server_number, decrypted_invite_byte);
      if (decrypted_invite_size == -1) {
        ERR_clear_error();
        delete[] encrypted_invite_byte;
        delete[] decrypted_invite_byte;
        command_queries =
            "SIGNAL SQLSTATE 'HY000' SET MYSQL_ERRNO='1003', MESSAGE_TEXT = "
            "'Decrypting using private key failed';";
        LogPluginErr(ERROR_LEVEL, ER_REWRITER_QUERY_FAILED,
                     "Decrypting using private key failed!");
        return 1;
      }

      decrypted_invite_byte[decrypted_invite_size] = '\0';
      std::string decrypted_invite_str(
          reinterpret_cast<char *>(decrypted_invite_byte));

      DBUG_PRINT(LOG_TAG, ("add_table_invite: decrypted_invite_str = %s",
                           decrypted_invite_str.c_str()));

      // different logic for LOCAL and SHARED tables
      // if table is LOCAL table
      if (decrypted_invite_str.find(",\"table_schema\":") !=
          std::string::npos) {
        DBUG_PRINT(LOG_TAG, ("add_table_invite: table is LOCAL"));

        // federated table obj
        trustdble::FEDERATED_TABLE fed_table;

        // read invite from invite string
        if (!parse_local_table_invite(decrypted_invite_str.c_str(),
                                      fed_table)) {
          delete[] encrypted_invite_byte;
          delete[] decrypted_invite_byte;
          LogPluginErr(ERROR_LEVEL, ER_REWRITER_QUERY_FAILED,
                       "Reading invite string failed!");
          return 1;
        }

        // create FEDERATED table
        std::string shared_db_str(reinterpret_cast<const char*>(shared_db));
        std::string create_smth = "CREATE TABLE " +
            shared_db_str+"."+fed_table.table_name + " (" + fed_table.table_schema + ") " +
            "ENGINE=FEDERATED CONNECTION='mysql://" + fed_table.user_name + ":" + fed_table.password +
            "@" + fed_table.hostname + ":" + fed_table.port + "/" + fed_table.db_name +
            "/" + fed_table.table_name + "';";
        DBUG_PRINT(LOG_TAG, ("add_table_invite: create_smth = %s", create_smth.c_str()));

        TableService tablereader;
        int error = tablereader.query(create_smth);

        if(error == 1) {
          command_queries = "select 'Create FEDERATED table failed' as '';";
          LogPluginErr(ERROR_LEVEL, ER_REWRITER_QUERY_FAILED,
            "could not create FEDERATED table");
          delete[] encrypted_invite_byte;
          delete[] decrypted_invite_byte;
          return 1;
        } else {
          command_queries = "select 'Successfully create FEDERATED table' as '';";
          DBUG_PRINT(LOG_TAG, ("Successfully create FEDERATED table"));
        }
      }
      // if table is SHARED table
      else {
        DBUG_PRINT(LOG_TAG, ("add_table_invite: table is SHARED"));

        // shared database obj
        trustdble::KEY_STORE key_store;
        // read invite from invite string
        if (!parse_table_invite(decrypted_invite_str.c_str(), key_store)) {
          delete[] encrypted_invite_byte;
          delete[] decrypted_invite_byte;
          LogPluginErr(ERROR_LEVEL, ER_REWRITER_QUERY_FAILED,
                       "Reading invite string failed!");
          return 1;
        }

        std::string shared_db_str(reinterpret_cast<const char *>(shared_db));
        if (key_store.db_name.compare(shared_db_str) != 0) {
          delete[] encrypted_invite_byte;
          delete[] decrypted_invite_byte;
          command_queries =
              "SIGNAL SQLSTATE 'HY000' SET MYSQL_ERRNO='1005', MESSAGE_TEXT = "
              "'Table does not belong to the current database';";
          LogPluginErr(ERROR_LEVEL, ER_REWRITER_QUERY_FAILED,
                       "Table does not belong to the current database");
          return 1;
        }
        trustdble::KEY_STORE table;
        command_queries = "";

        if (getKeyStore(shared_db, key_store.table_name.c_str(), table) == 1) {
          // insert values into key_store table
          std::string insert_stmt =
            "INSERT INTO " + std::string(shared_db) + "." + KEY_STORE_TABLE_NAME +
            "(tablename, encryptionkey, iv) values ('" + key_store.table_name +
            "','" + key_store.encryption_key + "','" + key_store.iv + "'" + ");";

          TableService tablereader;
          int error = tablereader.query(insert_stmt);

          if(error == 1) {
            command_queries = "select 'Insert values into key_store table failed' as '';";
              LogPluginErr(ERROR_LEVEL, ER_REWRITER_QUERY_FAILED,
              "could not insert values into key_store table");
            delete[] encrypted_invite_byte;
            delete[] decrypted_invite_byte;
            return 1;
          } else {
            command_queries = "select 'Successfully insert values into key_store table' as '';";
            DBUG_PRINT(LOG_TAG, ("Successfully insert values into key_store table"));
          }
        }
        else {
          std::string update_stmt =
            "UPDATE " + std::string(shared_db) + "." + KEY_STORE_TABLE_NAME +
            " SET encryptionkey='" + key_store.encryption_key + "'" +
            " ,iv='" + key_store.iv + "' WHERE tablename='" + key_store.table_name + "';";

          TableService tablereader;
          int error = tablereader.query(update_stmt);

          if(error == 1) {
            command_queries = "select 'UPDATE values into key_store table failed' as '';";
              LogPluginErr(ERROR_LEVEL, ER_REWRITER_QUERY_FAILED,
              "could not UPDATE values into key_store table");
            delete[] encrypted_invite_byte;
            delete[] decrypted_invite_byte;
            return 1;
          } else {
            command_queries = "select 'Successfully UPDATE values into key_store table' as '';";
            DBUG_PRINT(LOG_TAG, ("Successfully UPDATE values into key_store table"));
          }
        }
      }

      delete[] encrypted_invite_byte;
      delete[] decrypted_invite_byte;
      break;
    }
    case TrustdbleSharedCommand::create_data_contract: {
      DBUG_PRINT(LOG_TAG,
                 ("trustdble_command_method_call: create_data_contract"));

      std::string name = parameter_Set["name"].result;
      std::string owner = parameter_Set["owner"].result;
      std::string user = parameter_Set["user"].result;
      std::string logic_path = parameter_Set["logic_path"].result;
      std::string parameters = parameter_Set["parameters"].result;
      std::ifstream input_file(logic_path);
      if (!input_file.is_open()) {
        DBUG_PRINT(LOG_TAG, ("Could not open the logic file"));
        command_queries =
            "SIGNAL SQLSTATE 'HY000' SET MYSQL_ERRNO='1006', MESSAGE_TEXT = "
            "'Could not open the logic file';";
        return 1;
      }
      std::string logic =
          std::string((std::istreambuf_iterator<char>(input_file)),
                      std::istreambuf_iterator<char>());
      // replace all " in logic with \\" as in Json string we can not have
      // absolute " character
      std::string output;
      output.reserve(logic.size());
      for (const char c : logic) {
        switch (c) {
          case '"':
            output += "\\\"";
            break;
          case '\n':
            output += "\\n";
            break;
          case '\\':
            output += "\\\\";
            break;
          default:
            output += c;
            break;
        }
      }
      logic = output;
      command_queries = "";
      // create table encrypted_data_contract
      std::string create_stmt = " CREATE TABLE IF NOT EXISTS trustdble." +
                                DATA_CONTRACTS_NAME + " " +
                                DATA_CONTRACTS_SCHEMA + ";";
      TableService tablereader;
      int error = tablereader.query(create_stmt);

      if(error == 1) {
        command_queries = "SIGNAL SQLSTATE 'HY000' SET MYSQL_ERRNO='1006', MESSAGE_TEXT = "
        "'Creating data_contract table failed';";
          LogPluginErr(ERROR_LEVEL, ER_REWRITER_QUERY_FAILED,
          "Could not create data_contracts table");
        return 1;
      }

      // Insert data contract into data_contracts table
      std::string insert_stmt = "INSERT INTO trustdble." + DATA_CONTRACTS_NAME +
                                " VALUES ('" + name + "','" + owner + "','" +
                                user + "','" + logic + "','" + parameters +
                                "');";
      error = tablereader.query(insert_stmt);

       if(error == 1) {
        command_queries = "SIGNAL SQLSTATE 'HY000' SET MYSQL_ERRNO='1006', MESSAGE_TEXT = "
        "'Insert data contract into data_contracts table failed';";
          LogPluginErr(ERROR_LEVEL, ER_REWRITER_QUERY_FAILED,
          "Could not insert data contract into data_contract table");
        return 1;
      }

      command_queries = "select 'Data Contract successfully created' as '';";
      DBUG_PRINT(LOG_TAG, ("Data Contract successfully created"));

      break;
    }
    case TrustdbleSharedCommand::add_data_contract: {
      DBUG_PRINT(LOG_TAG, ("trustdble_command_method_call: add_data_contract"));
      ENCRYPTED_STRUCT encrypted_data_contract;
      // extract encrypted data contract in json format {"data_contract":""}
      std::string encrypted_data_contract_string =
          parameter_Set["encrypted_data_contract"].result;

      if (encrypted_data_contract_string != "") {
        DBUG_PRINT(LOG_TAG, ("Rewrite-Plugin: Read encrypted data contract "
                             "from encrypted data contract string: %s",
                             encrypted_data_contract_string.c_str()));
        // read encrypted data contract from data contract string
        if (!parse_encrypted_data_contract(
                encrypted_data_contract_string.c_str(),
                encrypted_data_contract)) {
          LogPluginErr(ERROR_LEVEL, ER_REWRITER_QUERY_FAILED,
                       "Reading encrypted data contract string failed!");
          command_queries =
              "SIGNAL SQLSTATE 'HY000' SET MYSQL_ERRNO='1008', MESSAGE_TEXT = "
              "'Encrypted data contract file is not in the right format';";
          return 1;
        }
      } else {
        DBUG_PRINT(LOG_TAG,
                   ("Rewrite-Plugin: Encrypted Data Contract is empty"));
        command_queries =
            "SIGNAL SQLSTATE 'HY000' SET MYSQL_ERRNO='1009', MESSAGE_TEXT = "
            "'Encrypted data contract is empty';";
        return 1;
      }

      DBUG_PRINT(LOG_TAG, ("Rewrite-Plugin: %s",
                           encrypted_data_contract_string.c_str()));

      std::string encrypted_key = encrypted_data_contract.encrypted_key;
      std::string encrypted_iv = encrypted_data_contract.encrytpted_iv;
      std::string encrypted_data_contract_str =
          encrypted_data_contract.encrypted_string;
      unsigned char *encrypted_key_byte =
          new unsigned char[encrypted_key.length() / 2];
      unsigned char *encrypted_iv_byte =
          new unsigned char[encrypted_iv.length() / 2];
      unsigned char *encrypted_data_contract_byte =
          new unsigned char[encrypted_data_contract_str.length() / 2];
      unsigned char *decrypted_data_contract_byte =
          new unsigned char[encrypted_data_contract_str.length() / 2];
      unsigned char *decrypted_key = new unsigned char[KEY_SIZE];
      unsigned char *decrypted_iv = new unsigned char[IV_SIZE];
      int key_error = hexToCharArray(encrypted_key, encrypted_key_byte);
      int iv_error = hexToCharArray(encrypted_iv, encrypted_iv_byte);
      int data_contract_error = hexToCharArray(encrypted_data_contract_str,
                                               encrypted_data_contract_byte);
      if (key_error == 1 || iv_error == 1 || data_contract_error == 1) {
        command_queries =
            "SIGNAL SQLSTATE 'HY000' SET MYSQL_ERRNO='1007', MESSAGE_TEXT = "
            "'Encrypted file is not in the right format';";
        delete[] encrypted_key_byte;
        delete[] encrypted_iv_byte;
        delete[] encrypted_data_contract_byte;
        delete[] decrypted_data_contract_byte;
        delete[] decrypted_key;
        delete[] decrypted_iv;
        return 1;
      }
      // decrypt encrypted key/iv using private key
      int decrypted_key_size =
          decrypt_private(encrypted_key_byte, encrypted_key.length() / 2,
                          config.config_configuration_path,
                          config.config_server_number, decrypted_key);
      int decrypted_iv_size =
          decrypt_private(encrypted_iv_byte, encrypted_iv.length() / 2,
                          config.config_configuration_path,
                          config.config_server_number, decrypted_iv);

      if (decrypted_key_size == -1 || decrypted_iv_size == -1) {
        ERR_clear_error();
        delete[] encrypted_key_byte;
        delete[] encrypted_iv_byte;
        delete[] encrypted_data_contract_byte;
        delete[] decrypted_data_contract_byte;
        delete[] decrypted_key;
        delete[] decrypted_iv;
        command_queries =
            "SIGNAL SQLSTATE 'HY000' SET MYSQL_ERRNO='1003', MESSAGE_TEXT = "
            "'Decrypting using private key failed';";
        LogPluginErr(ERROR_LEVEL, ER_REWRITER_QUERY_FAILED,
                     "Decrypting using private key failed!");
        return 1;
      }
      // decrypt encrypted data contract using key/iv
      size_t decrypted_data_contract_len =
          decrypt(encrypted_data_contract_byte,
                  encrypted_data_contract_str.length() / 2, decrypted_key,
                  decrypted_iv, decrypted_data_contract_byte);
      // Error handling when decryption fails
      if (decrypted_data_contract_len == 0) {
        command_queries =
            "SIGNAL SQLSTATE 'HY000' SET MYSQL_ERRNO='1006', MESSAGE_TEXT = "
            "'Decryption failed';";
        delete[] encrypted_key_byte;
        delete[] encrypted_iv_byte;
        delete[] encrypted_data_contract_byte;
        delete[] decrypted_data_contract_byte;
        delete[] decrypted_key;
        delete[] decrypted_iv;
        return 1;
      }
      decrypted_data_contract_byte[decrypted_data_contract_len] = '\0';
      std::string decrypted_data_contract_str(
          reinterpret_cast<char *>(decrypted_data_contract_byte));
      REMOTE_DATA_CONTRACT decrypted_data_contract;
      // read data contract from data contract string
      if (!parse_data_contract(decrypted_data_contract_str.c_str(),
                               decrypted_data_contract)) {
        delete[] encrypted_key_byte;
        delete[] encrypted_iv_byte;
        delete[] encrypted_data_contract_byte;
        delete[] decrypted_data_contract_byte;
        delete[] decrypted_key;
        delete[] decrypted_iv;
        LogPluginErr(ERROR_LEVEL, ER_REWRITER_QUERY_FAILED,
                     "Reading data contract string failed!");
        return 1;
      }

      // create data contracts table
      command_queries = "";
      std::string create_stmt = "CREATE TABLE IF NOT EXISTS trustdble." +
                                REMOTE_DATA_CONTRACTS_NAME + " " +
                                REMOTE_DATA_CONTRACTS_SCHEMA + ";";
                                 DATA_CONTRACTS_SCHEMA + ";";
      TableService tablereader;
      int error = tablereader.query(create_stmt);

      if(error == 1) {
        command_queries = "SIGNAL SQLSTATE 'HY000' SET MYSQL_ERRNO='1006', MESSAGE_TEXT = "
        "'Creating remote_data_contract table failed';";
          LogPluginErr(ERROR_LEVEL, ER_REWRITER_QUERY_FAILED,
          "Could not create remote_data_contracts table");
        delete[] encrypted_key_byte;
        delete[] encrypted_iv_byte;
        delete[] encrypted_data_contract_byte;
        delete[] decrypted_data_contract_byte;
        delete[] decrypted_key;
        delete[] decrypted_iv;
        return 1;
      }

      // insert values into data contracts table (name of data contract is
      // data_contracts' table Unique ID)
       std::string output;
      output.reserve(decrypted_data_contract.logic.size());
      for (const char c : decrypted_data_contract.logic) {
        switch (c) {
          case '\\':// escape / character
            output += "\\\\";
            break;
          default:
            output += c;
            break;
        }
      }
      decrypted_data_contract.logic = output;
      std::string insert_stmt = "INSERT INTO trustdble." + REMOTE_DATA_CONTRACTS_NAME +
                                " VALUES ('" + decrypted_data_contract.name +
                                "','" + decrypted_data_contract.owner +
                                "','" + decrypted_data_contract.user +
                                "','" + decrypted_data_contract.logic +
                                "','" + decrypted_data_contract.parameters +
                                "','" +
                                decrypted_data_contract.server_address + "','" +
                                decrypted_data_contract.MRSIGNER + "','" +
                                decrypted_data_contract.MRENCLAVE + "','" +
                                decrypted_data_contract.isv_prod_id + "','" +
                                decrypted_data_contract.isv_svn + "');";

                                 DATA_CONTRACTS_SCHEMA + ";";
      error = tablereader.query(insert_stmt);

      if(error == 1) {
        command_queries = "SIGNAL SQLSTATE 'HY000' SET MYSQL_ERRNO='1006', MESSAGE_TEXT = "
        "'Insert data contract into remote_data_contract table failed';";
          LogPluginErr(ERROR_LEVEL, ER_REWRITER_QUERY_FAILED,
          "Could not insert data contract into remote_data_contracts table");
        delete[] encrypted_key_byte;
        delete[] encrypted_iv_byte;
        delete[] encrypted_data_contract_byte;
        delete[] decrypted_data_contract_byte;
        delete[] decrypted_key;
        delete[] decrypted_iv;
        return 1;
      }

      command_queries = "select 'Data Contract successfully added' as '';";
      DBUG_PRINT(LOG_TAG, ("Data Contract successfully added"));


      delete[] encrypted_key_byte;
      delete[] encrypted_iv_byte;
      delete[] encrypted_data_contract_byte;
      delete[] decrypted_data_contract_byte;
      delete[] decrypted_key;
      delete[] decrypted_iv;
      break;
    }
    case TrustdbleSharedCommand::encrypt_data_contract: {
      DBUG_PRINT(LOG_TAG,
                 ("trustdble_command_method_call: encrypt_data_contract"));

      std::string name = parameter_Set["name"].result;
      // get IP Address
      char hostname[1024];
      hostname[1023] = '\0';
      gethostname(hostname, 1023);
      struct hostent *host_entry;
      host_entry = gethostbyname(hostname);
      char *ip_address = inet_ntoa(*(struct in_addr *)*host_entry->h_addr_list);

      DATA_CONTRACT data_contract;
      if (trustdble::getDataContract(name, data_contract) != 0) {
        DBUG_PRINT(LOG_TAG, ("Rewrite-Plugin:Specified Data Contract does not exist"));
        command_queries =
            "SIGNAL SQLSTATE 'HY000' SET MYSQL_ERRNO='1005', MESSAGE_TEXT = "
            "'Rewrite-Plugin:Specified Data Contract does not exist';";
        return 1;
      }
      SGX_MEASUREMENT sgx_measurement;
        if (trustdble::getSgxMeasurement(sgx_measurement) != 0) {
          DBUG_PRINT(LOG_TAG, ("Rewrite-Plugin:SGX measurements are not provided"));
          command_queries =
            "SIGNAL SQLSTATE 'HY000' SET MYSQL_ERRNO='1006', MESSAGE_TEXT = "
            "'Rewrite-Plugin:SGX measurements are not provided';";
            return 1;
        }
      // replace all " in logic with \\" as in Json string we can not have
      // absolute " character
      std::string output;
      output.reserve(data_contract.logic.size());
      for (const char c : data_contract.logic) {
        switch (c) {
          case '"':
            output += "\\\"";
            break;
          case '\n':
            output += "\\n";
            break;
          case '\\':
            output += "\\\\";
            break;
          default:
            output += c;
            break;
        }
      }
      std::string logic;
      logic=output;
      std::string data_contract_string =
          "{\"name\":\"" + name + "\",\"owner\":\"" + data_contract.owner +
          "\",\"user\":\"" + data_contract.user + "\",\"logic\":\"" + logic +
          "\",\"parameters\":\"" + data_contract.parameters + "\",\"server_address\":\"" +
          ip_address + "\",\"MRSIGNER\":\"" + sgx_measurement.MRSIGNER +
          "\",\"MRENCLAVE\":\"" + sgx_measurement.MRENCLAVE +
          "\",\"isv_prod_id\":\"" + sgx_measurement.isv_prod_id +
          "\",\"isv_svn\":\"" + sgx_measurement.isv_svn + "\"}";
      const unsigned char *data_contract_byte =
          (const unsigned char *)data_contract_string.c_str();

      std::string public_key_primary = parameter_Set["public_key"].result;
      std::string start_public = "-----BEGIN PUBLIC KEY-----\n";
      std::string end_public = "\n-----END PUBLIC KEY-----\n";
      std::string public_key =
          start_public.append(public_key_primary).append(end_public);
      const char *public_key_byte = public_key.c_str();
      int size = public_key_primary.length();
      if (size == 0) {
      }

      if (!is_base64_encoded(public_key_primary)) {
        DBUG_PRINT(LOG_TAG, ("public key is not valid"));
        command_queries =
            "SIGNAL SQLSTATE 'HY000' SET MYSQL_ERRNO='1003', MESSAGE_TEXT = "
            "'Public key is not valid';";
        return 1;
      }
      if (!validate_publickey(public_key_byte)) {
        ERR_clear_error();
        DBUG_PRINT(LOG_TAG, ("public key is not valid"));
        command_queries =
            "SIGNAL SQLSTATE 'HY000' SET MYSQL_ERRNO='1003', MESSAGE_TEXT = "
            "'Public key is not valid';";
        return 1;
      }
      command_queries = "";
      if (!data_contract_string.empty() && !public_key_primary.empty()) {
        // the maximum size of data that can be encrypted using public key is
        // the size of public key as most of data contract strings size is more
        // than 512 bytes so:
        // 1. encrypt data contract using symmetric key
        // 2. encrypt symmetric key and iv using public key
        size_t data_contract_len = data_contract_string.length();
        // generate symmetric key and iv
        unsigned char *encryption_key = new unsigned char[KEY_SIZE];
        unsigned char *iv = new unsigned char[IV_SIZE];
        RAND_bytes(encryption_key, KEY_SIZE);
        RAND_bytes(iv, IV_SIZE);
        // encrypt data contract string using symmetric encryption
        unsigned char *encrypted_data_contract =
            new unsigned char[data_contract_len + 256];
        size_t encrypted_data_contract_len =
            encrypt(data_contract_byte, data_contract_len, encryption_key, iv,
                    encrypted_data_contract);
        // encrypt symmetric key and iv using public key
        BYTES encrypted_key, encrypted_iv;
        encrypted_key =
            encrypt_public(encryption_key, KEY_SIZE, public_key_byte);
        encrypted_iv = encrypt_public(iv, IV_SIZE, public_key_byte);
        // convert encrypted symmetric key/iv and encrypted data contract into
        // hex
        std::string encrypted_data_contract_str = byte_array_to_hex(
            encrypted_data_contract, encrypted_data_contract_len);
        std::string encrypted_key_str =
            byte_array_to_hex(encrypted_key.value, encrypted_key.size);
        std::string encrypted_iv_str =
            byte_array_to_hex(encrypted_iv.value, encrypted_iv.size);

        // create table encrypted_data_contract
        std::string create_stmt = " CREATE TABLE IF NOT EXISTS trustdble." +
                                  ENCRYPTED_DATA_CONTRACTS_NAME + " " +
                                  ENCRYPTED_DATA_CONTRACTS_SCHEMA + ";";
        TableService tablereader;
        int error = tablereader.query(create_stmt);

        if(error == 1) {
          command_queries = "SIGNAL SQLSTATE 'HY000' SET MYSQL_ERRNO='1006', MESSAGE_TEXT = "
          "'Creating encrypted_data_contracts table failed';";
            LogPluginErr(ERROR_LEVEL, ER_REWRITER_QUERY_FAILED,
            "Could not create encrypted_data_contracts table");
          delete[] encrypted_data_contract;
          delete[] encryption_key;
          delete[] iv;
          return 1;
        }

        // insert encrypted data contract into encrypted_data_contracts table
        std::string insert_stmt =
            "INSERT INTO trustdble." + ENCRYPTED_DATA_CONTRACTS_NAME +
            " VALUES ('" + name + "','" + encrypted_key_str + "','" +
            encrypted_iv_str + "','" + encrypted_data_contract_str + "');";
        error = tablereader.query(insert_stmt);

        if(error == 1) {
          command_queries = "SIGNAL SQLSTATE 'HY000' SET MYSQL_ERRNO='1006', MESSAGE_TEXT = "
          "'Insert into encrypted_data_contracts table failed';";
            LogPluginErr(ERROR_LEVEL, ER_REWRITER_QUERY_FAILED,
            "Could not insert into encrypted_data_contracts table");
          delete[] encrypted_data_contract;
          delete[] encryption_key;
          delete[] iv;
          return 1;
        }

        DBUG_PRINT(LOG_TAG,
                   ("Rewrite-Plugin: %s", encrypted_data_contract_str.c_str()));
        delete[] encrypted_data_contract;
        delete[] encryption_key;
        delete[] iv;
      } else {
        DBUG_PRINT(LOG_TAG, ("Rewrite-Plugin: Read Data Contract failed OR "
                             "Public key is empty"));
        return 1;
      }
      command_queries = "select 'Data Contract successfully encrypted' as '';";
      DBUG_PRINT(LOG_TAG, ("Data Contract successfully encrypted"));

      break;
    }
  }
  return 0;
}
