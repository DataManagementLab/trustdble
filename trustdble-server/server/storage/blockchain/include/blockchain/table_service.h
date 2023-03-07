#ifndef TABLE_SERVICE
#define TABLE_SERVICE

#include <mysql.h>
#include <string>
#include <vector>
#include "sql/table.h"

namespace trustdble {

/**
 * @brief Struct represeting a TrustDBle invite or the schema of the
 * trustdble.shared_databases table. That can be used by another TrustDBle
 * instance to join a shared database or used for reading the schemas of shared
 * databases.
 *
 * @details A TrustDBle invite is used to allow other parties to connect and use
 * a shared database. It contains connection information that are required to
 * connect to a meta-data blockchain. These connection information are
 * blockchain specific and are defined in the chain_config parameter. Upon
 * Creation of a shared database this Information is stored in the
 * trustdble.shared_databases table.
 *
 * @param name Name of the shared database
 * @param address Table address of the meta table
 * @param meta_chain_config Configuration parameters of the meta blockchain
 * @param bc_type Blockchain-Type of the meta blockchain
 * @param encryption_key encryption_key database
 */
struct SHARED_DATABASE {
  std::string name;
  std::string meta_address;
  std::string meta_chain_config;
  std::string bc_type;
  std::string encryption_key;
  std::string iv;
};

/**
 * @brief Struct represeting a TrustDBle encrypted invite.
 * That can be used by another TrustDBle instance to decrypt the encrypted
 * invite file to join a shared database.
 *
 * @details A TrustDBle encrypted STRUCT is used to allow other parties to decrypt the encrypted string using encrypted key/iv
 * It contains encrypted information which are used to decrypt the encrypted string.
 *
 * @param key Encrypted symmetric key which is used to encrypt the file (is encrypted using public key)
 * @param iv Encrypted iv which is used to encrypt the file (is encrypted using public key)
 * @param encrypted_string Encrypted string which is encrypted using key/iv
 */
struct ENCRYPTED_STRUCT{
  std::string encrypted_key;
  std::string encrytpted_iv;
  std::string encrypted_string;
};

/**
 * @brief Struct that is representing a row in the meta_table of a shared
 database.

 * It stores name, address and schema of a shared table.

 */
struct SHARED_TABLE {
  std::string name;
  std::string address;
  std::string schema;
};

/**
 * @brief Struct that is representing a row in the key_store of a shared table.

 * It stores table name, encryption key and iv of a shared table.

 */
struct KEY_STORE {
  std::string db_name;
  std::string table_name;
  std::string encryption_key;
  std::string iv;
};

/**

 * @brief Struct that is representing a data contract.

 * It stores remote data contract name, data owner, data user, logic , parameters and server address and enclave measurments(MRSIGNEr,MRNCLAVE,isv_prod_id;
  isv_svn) of data contract.
 */
struct REMOTE_DATA_CONTRACT {
  std::string name;
  std::string owner;
  std::string user;
  std::string logic;
  std::string parameters;
  std::string server_address;
  std::string MRSIGNER;
  std::string MRENCLAVE;
  std::string isv_prod_id;
  std::string isv_svn;
};

/**
 * @brief This class provides methods to access the tables without using a SQL queries.
 * @brief Struct that is representing a connection information of a FEDERATED LOCAL table.

 * It stores db_name, table_name, table_schema, hostname, port, user_name, password of a FEDERATED LOCAL table.

 */
struct FEDERATED_TABLE {
  std::string db_name;
  std::string table_name;
  std::string table_schema;
  std::string hostname;
  std::string port;
  std::string user_name;
  std::string password;
};
/**
 * @brief Struct that is representing a SGX meaurement which is used for verification.

 * It stores MRSIGNER, MRENCLAVE, isv_prod_id, isv_svn.

 */
struct SGX_MEASUREMENT {
  std::string MRSIGNER;
  std::string MRENCLAVE;
  std::string isv_prod_id;
  std::string isv_svn;
};

/**
 * @brief Struct that is representing a Data Contract.

 * It stores name, owner, user, logic and parameters.

 */
struct DATA_CONTRACT {
  std::string name;
  std::string owner;
  std::string user;
  std::string logic;
  std::string parameters;
};

/**
 * @brief This class provides methods to run SQL statements on the local MySQL server.
 */
class TableService{
    public:

  /**
   * @brief Execute any SQL query (INSERT, UPDATE, SELECT ...)
   *
   * @param query The query to be executed as string
   * @return 0 if successful otherwise 1
   */
  auto query(const std::string &query, const std::string &database_name = "", const std::string &table_name = "") -> int;

  auto query_prepared_stmt(const std::string &query,const std::string &database_name, std::vector<std::string> parameter_queries) -> int;

  /**
   * @brief Initialize reading the result of the previous executed query.
   *
   * @return 0 if successful otherwise 1
   */
  auto readInit() -> int;

  /**
   * @brief Reads the current row of the result set
   *
   * @return 0 if successful otherwise 1
   */
  auto readNext() -> int;

  /**
   * @brief Reads the current selected row into a string vector
   *
   * @param row Result vector to store all fields of the current row
   * @return int
   */
  auto readRow(std::vector<std::string> &row) -> int;

  /**
   * @brief Finishes reading, frees allocated memory and closes connection
   *
   * @return 0 if successful otherwise 1
   */
  auto readEnd() -> int;

  /**
   * @brief escapes the given input string
   *
   * @param pStr a pointer to the given string
   * @return the escaped string
   */
  auto escapeInput(const char *pStr) ->std::string;

 private:
  // member variables
  std::string m_username;
  std::string m_password;
  std::string m_host;
  int m_port;
  MYSQL *m_con;
  MYSQL_RES *m_result;
  MYSQL_ROW m_row;
  size_t m_num_fields;

   // HELPER METHODS //
  /**
   * @brief Initialize the client connection
   *
   * @return 0 if successful otherwise 1
   */
  auto init() -> int;
};

const std::string META_TABLE_NAME = "meta_table";
const std::string META_TABLE_SCHEMA =
    "(tablename VARCHAR(50), tableaddress VARCHAR(255), tableschema VARCHAR(2100))";
const std::string KEY_STORE_TABLE_NAME = "key_store";
const std::string KEY_STORE_TABLE_SCHEMA =
    "(tablename VARCHAR(50), encryptionkey VARCHAR(64), iv VARCHAR(32), PRIMARY KEY (tablename, encryptionkey, iv))";
const std::string ENGINE_STRING = " ENGINE=BLOCKCHAIN";
// table data_chains to store information about data chains
const std::string META_TABLE_DATA_CHAINS_NAME = "data_chains";
const std::string META_TABLE_DATA_CHAINS_SCHEMA =
    "(datachain_id INT, connection_information VARCHAR(1000))";
// table shared_tables to store
// partition_id-table_address-table_schema-encryption_key-iv
const std::string SHARED_TABLES_NAME = "shared_tables";
// first_column is (PK) partition_id = table_name [VARCHAR(50)] + '/' +
// data_chain_id [INT]
const std::string SHARED_TABLES_SCHEMA =
    "(partition_id VARCHAR(50), tableaddress VARCHAR(255), tableschema VARCHAR(2100))";
// table encrypted_invites to store encrypted invite strings
const std::string ENCRYPTED_INVITE_NAME = "encrypted_invite";
const std::string ENCRYPTED_INVITE_SCHEMA =
    "(id VARCHAR(737), database_name VARCHAR(100),encrypted_key TEXT, encrypted_iv TEXT, encrypted_invite TEXT, PRIMARY KEY (id,database_name)) CHARACTER SET utf8";
// table inivites to store invite
const std::string INVITES_NAME = "invites";
const std::string INVITES_SCHEMA =
    "(database_name VARCHAR(100), blockchain_type ENUM(\'STUB\', \'ETHEREUM\', \'FABRIC\'), "
    "meta_table_address VARCHAR(200), meta_chain_config JSON, encryption_key VARCHAR(64),iv VARCHAR(32),PRIMARY KEY (database_name))";

 // table encrypted_table_invite to store encrypted table invite strings
const std::string ENCRYPTED_TABLE_INVITE_NAME = "encrypted_table_invite";
const std::string ENCRYPTED_TABLE_INVITE_SCHEMA =
    "(table_name VARCHAR(50), encrypted_invite TEXT)";
  
// table data contracts to store decrypted remote data contract
const std::string REMOTE_DATA_CONTRACTS_NAME = "remote_data_contracts";
const std::string REMOTE_DATA_CONTRACTS_SCHEMA =
    "(name VARCHAR(100), owner VARCHAR(100), user VARCHAR(100), logic TEXT, parameters VARCHAR(100), server_address VARCHAR(100),MRSIGNER VARCHAR(64), MRENCLAVE VARCHAR(64), isv_prod_id VARCHAR(2), isv_svn VARCHAR(2), PRIMARY KEY (name))";

// table data contracts to store local data contract
const std::string DATA_CONTRACTS_NAME = "data_contracts";
const std::string DATA_CONTRACTS_SCHEMA =
    "(name VARCHAR(100), owner VARCHAR(100), user VARCHAR(100), logic TEXT, parameters VARCHAR(100), PRIMARY KEY (name))";

// table data contracts to store data contracts
const std::string ENCRYPTED_DATA_CONTRACTS_NAME = "encrypted_data_contracts";
const std::string ENCRYPTED_DATA_CONTRACTS_SCHEMA =
    "(name VARCHAR(100),encrypted_key TEXT, encrypted_iv TEXT, encrypted_data_contract TEXT,PRIMARY KEY (name))";

const std::string META_APPENDIX_CONNECTION = "connection";
const std::string META_APPENDIX_STATEMENT = "statement";
// TODO remove after replacing code with create_network_name()
const std::string NETWORK_NAME_PREFIX_META = "meta";
const std::string NETWORK_NAME_PREFIX_DATA = "data";

/**
 * @brief  Method to read from local shared database table and get meta data of
 * database with a given name.
 *
 * @param shared_db object to store meta data
 * @param databasename pointer referring to the database name
 * @return 0 if successful, otherwise 1
 */
auto getSharedDatabase(SHARED_DATABASE &shared_db,
                       const char *databasename) -> int;

/**
 * @brief Get a specific shared table from the meta_table of a specific shared
 * database
 *
 * @param[in] shared_db Name of the shared database
 * @param[in] tablename Name of the shared table
 * @param[out] table Output parameter to store read shared table
 * @return 0 if successful otherwise 1
 */
auto getSharedTable(const char *shared_db, const char *tablename,
                    SHARED_TABLE &table) -> int;

/**
 * @brief check if a shared table exists in the shared_tables
 *
 * @param[in] databasename Name of the shared database
 * @param[in] tablename Name of the shared table
 * @return 0 if successful otherwise 1
 */
auto checkTable(const char *db_name,
                const std::string &tablename, const int shard_number) -> bool;

/**
 * @brief  Method to read schemas from meta-table of the current shared
 * database.
 *
 * @param tableInformationList Vector of tuples of strings to store tablename
 * and tableschema  read from the meta-table
 *
 * @return 0 if successful, otherwise 1
 */
auto getMetaData(
    std::vector<std::tuple<std::string, std::string>> &tableInformationList,
    const char *shared_db) -> int;

/**
 * @brief  Method to read schemas from shared_tables of the current shared
 * database.
 *
 * @param tableInformationList Vector of tuples of strings to store tablename
 * and tableschema read from the shared_tables
 *
 * @return 0 if successful, otherwise 1
 */
auto getMetaDataForSharedDataTable(
    std::vector<std::tuple<std::string, std::string>> &tableInformationList,
    const char *shared_db) -> int;

/**
   * @brief Get a specific row from the key_store of a specific shared
   * table
   *
   * @param[in] shared_db Name of the shared database
   * @param[in] tablename Name of the shared table
   * @param[out] table Output parameter to store read row in key_store
   * @return 0 if successful otherwise 1
   */
auto getKeyStore(const char *shared_db,const char *tablename,KEY_STORE &table) -> int;


/**
 * @brief  Method to store all keys for the given table in the current shared
 * database.
 *
 * @param list_of_keys Vector of of strings to store keys read for the given
 * table
 *
 * @return 0 if successful, otherwise 1
 */
auto getListOfKeys(std::vector<std::string> &list_of_keys,
                   const char *table_name, const char *shared_db) -> int;
/**
 * @brief  Method to read from local invites table.
 *
 * @param invite object to store invite data
 * @return 0 if successful, otherwise 1
 */
auto getInvite(const char *databasename, SHARED_DATABASE &invite) -> int;

/**
 * @brief  Method to delete invite for database from invites table.
 *
 * @param databasename Name of the shared database
 * @return 0 if successful, otherwise 1
 */
auto deleteInvite(const char *databasename) -> int;


/**
   * @brief Get a specific row from the key_store of a specific shared
   * table
   *
   * @param[in] thd Current thread
   * @param[in] shared_db Name of the shared database
   * @param[in] tablename Name of the shared table
   * @param[out] table Output parameter to store read row in key_store
   * @return 0 if successful otherwise 1
   */

auto getKeyStore(const char *shared_db,const char *tablename,KEY_STORE &table) -> int;

/**
 * @brief  Method to read container_name from docker_container table of the
 * current shared database.
 *
 * @param thd Current thread
 * @param data_container_name Collection of strings to store container_name
 * information
 * @param shared_db Pointer referring to the database
 *
 * @return 0 if successful, otherwise 1
 */
auto get_container_name(std::vector<std::string> &data_container_name,
                              const char *shared_db) -> int;

/**
 * @brief  Method to read number of shards from data_chains table
 *
 * @param thd Current thread
 * @param shared_db Pointer referring to the database
 *
 * @return Number of shards
 */
auto getNumShards(const std::string &shared_db) -> int;

/**
 * @brief  Method to copy all data from table
 *
 * @param[in] thd Current thread
 * @param[in] shared_db Pointer referring to the database
 * @param[in] table_name Name of the table
 * @param[out] table_data Vector of vectors of strings to store data
 *
 * @return 0 if successful, otherwise 1
 */
auto copy_table (const char *shared_db, const char *table_name,
                       std::vector<std::vector<std::string>> &table_data) -> int;

/**
 * @brief  Method to read hostname.
 * @param[out] hostname hostname
 * @return 0 if successful, otherwise 1
 */
auto get_hostname (std::string &hostname) -> int;

/**
 * @brief  Method to read port.
 * @param[out] port port
 * @return 0 if successful, otherwise 1
 */
auto get_port (std::string &port) -> int;

/**
 * @brief  Method to read table_schema information for LOCAL table in database.
 * @param[out] table_schema Table schema
 * @param[in] db_name Database name
 * @param[in] table_name Table name
 * @return 0 if successful, otherwise 1
 */
auto get_local_table_schema (std::string &table_schema, const std::string &db_name, const std::string &table_name) -> int;

/**
 * @brief  Method to read table_name information about LOCAL tables in database.
 *
 * @param[out] list_of_tables Vector of strings to store table_name information
 * @param[in] db_name Database name
 *
 * @return 0 if successful, otherwise 1
 */
auto get_list_of_local_tables (std::vector<std::string> &list_of_tables, const std::string &db_name) -> int;

/**
 * @brief  Method to read table_name information about data tables from shared_tables table of the current shared database.
 *
 * @param list_of_tables Collection of strings to store table_name information
 * @param shared_db Pointer referring to the database
 *
 * @return 0 if successful, otherwise 1
 */
auto get_list_of_tables (std::vector<std::string> &list_of_tables, const char *shared_db) ->int ;

/**
 * @brief  Method to read data_network_config information about data-chains from data-chains table of the current shared database.
 *
 * @param thd Current thread
 * @param data_chains_network_config Collection of strings to store
 * data_network_config information
 * @param shared_db Pointer referring to the database
 *
 * @return 0 if successful, otherwise 1
 */
auto GetDataNetworkConfig(std::vector<std::string> &data_chains_network_config,
    const char *shared_db) -> int;

/**
 * @brief  Method to read data_network_config information for the shard_number from data_chains table of the current shared database
 *
 * @param[in] thd Current thread
 * @param[out] data_chains_network_config String to store data_network_config information
 * @param[in] database_name Name of the shared database
 * @param[in] shard_number Shard number
 *
 * @return 0 if successful, otherwise 1
 */
auto GetDataNetworkConfigForShard(std::string &data_chains_network_config,
                                             const std::string &database_name,
                                             const int shard_number) -> int;


/**
 * @brief Get schema of specific data table
 *
 * @param[in] table_name Name of the table
 * @param[out] table_schema Schema of the table
 * @param[in] shared_db Pointer referring to the database
 *
 * @return 0 if successful, otherwise 1
 */
auto get_table_schema(const std::string &table_name, std::string &table_schema,
                                     const char *shared_db) -> int;

/**
   * @brief Get a specific shared table schema from table shared_tables
   *
   * @param[in] databasename Name of the shared database
   * @param[in] tablename Name of the shared table
   * @param[out] table Output parameter to store read shared table
   * @return 0 if successful otherwise 1
   */
auto getSchemaForSharedDataTable(const std::string &databasename,
                                const std::string &tablename,
                                SHARED_TABLE &table) -> int;

  /**
   * @brief Get a specific shared table address from table shared_tables
   *
   * @param[in] databasename Name of the shared database
   * @param[in] tablename Name of the shared table
   * @param[in] shard_number data_chain_id
   * @param[out] table Output parameter to store read shared table
   * @return 0 if successful otherwise 1
   */
auto getAddressForSharedDataTable(const std::string &databasename,
                                const std::string &tablename,
                                const int shard_number,
                                SHARED_TABLE &table) -> int;
/**
 * @brief Get SGX measurement info 
 *
 * @param[in] sgx_measurement SGX measurement
 * @return 0 if successful otherwise 1
 */
auto getSgxMeasurement(SGX_MEASUREMENT &sgx_measurement) -> int;

/**
 * @brief Get local data contract info 
 *
 * @param[in] name Data Contract's name
 * @param[in] data_contract Data Contract
 * @return 0 if successful otherwise 1
 */
auto getDataContract(std::string name, DATA_CONTRACT &data_contract) -> int;
/**
 * @brief Update a shared database in the shared_database table of the
 * trustdble database
 *
 * @param[in] database Shared database struct with updated values
 * @return 0 if successful otherwise 1
 */
auto updateSharedDatabase(const SHARED_DATABASE &database) -> int;

/**
 * @brief Update a shared table in the meta_table table of a shared database
 *
 * @param[in] databasename Name of shared database
 * @param[in] table Shared table struct with updated values
 * @return 0 if successful otherwise 1
 */
auto updateSharedTable(const std::string &databasename,
                        const SHARED_TABLE &table) -> int;
/**
 * @brief Update a table table_addresses
 *
 * @param[in] databasename Name of shared database
 * @param[in] table Shared table struct with updated values
 * @param[in] shard_number shard number
 * @return 0 if successful otherwise 1
 */
auto updateSharedDataTableAddress(const std::string &databasename,
                                  const SHARED_TABLE &table,
                                  int shard_number) -> int;

} // namespace table_service
#endif // TABLE_SERVICE
