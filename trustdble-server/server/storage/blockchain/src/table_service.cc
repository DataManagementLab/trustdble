#include "blockchain/table_service.h"
#include "sql/mysqld.h"
#include <string>

using namespace trustdble;
// LOG-Tag for this class
#define LOG_TAG "table_service"
auto trustdble::getSharedDatabase(SHARED_DATABASE &shared_db,
                                  const char *databasename) -> int {

  if (!databasename) {
    return 1;
  }
  std::string query = "'SELECT * FROM shared_databases where database_name = ?'";
  std::vector<std::string> parameter_queries {"SET @1 = '"+ std::string(databasename)+"';"};

  std::vector<std::string> result_row;
  TableService tablereader;
  tablereader.query_prepared_stmt(query, "trustdble", parameter_queries);
  tablereader.readInit();
  tablereader.readNext();
  tablereader.readRow(result_row);
  tablereader.readEnd();

  if (!result_row.empty()) {
    shared_db.name = result_row.at(1);               // 1
    shared_db.bc_type = result_row.at(2);            // 2
    shared_db.meta_address = result_row.at(3);       // 3
    shared_db.meta_chain_config = result_row.at(4);  // 4
    shared_db.encryption_key = result_row.at(5);     // 5
    shared_db.iv = result_row.at(6);                 // 6
    return 0;
  }
  return 1;
}

auto trustdble::getSharedTable(const char *shared_db,
                               const char *tablename, SHARED_TABLE &table)-> int {

  if (!tablename | !shared_db) {
    return 1;
  }
  TableService tablereader;
  std::string query = "'SELECT * FROM meta_table where tablename = ?'";
  std::vector<std::string> parameter_queries {"SET @1 = '"+ std::string(tablename) +"';"};

  std::vector<std::string> result_row;
  tablereader.query_prepared_stmt(query, std::string(shared_db), parameter_queries);
  tablereader.readInit();
  tablereader.readNext();
  tablereader.readRow(result_row);
  tablereader.readEnd();

  if (!result_row.empty()) {
    table.name = result_row.at(0);           // 0
    table.address = result_row.at(1);        // 1
    table.schema = result_row.at(2);         // 2
    return 0;
  }
  return 1;
}

auto trustdble::checkTable(const char *db_name,
                           const std::string &tablename, const int shard_number)-> bool {
  // get partition_id = table_name + '/' + data_chain_id
  const std::string partition_id =
      tablename + '/' + std::to_string(shard_number);

  if (!db_name) {
    return 1;
  }
  std::string query = "'SELECT * FROM shared_tables where partition_id = ?'";

  std::vector<std::string> parameter_queries {"SET @1 = '"+ partition_id+"';"};

  std::vector<std::string> result_row;
  TableService tablereader;
  tablereader.query_prepared_stmt(query, std::string(db_name), parameter_queries);
  tablereader.readInit();
  tablereader.readNext();
  tablereader.readRow(result_row);
  tablereader.readEnd();

  if (!result_row.empty()) {
    return true;
  }
  return false;
}

auto trustdble::getMetaData(
        std::vector<std::tuple<std::string, std::string>> &tableInformationList,
    const char *shared_db) -> int {

  if (!shared_db) {
    return 1;
  }
  TableService tablereader;
  std::string query = "'SELECT tablename, tableschema FROM '";

  std::vector<std::string> result_row;
  tablereader.query(query, shared_db, "meta_table");
  tablereader.readInit();

  // read multiple_rows
  while(tablereader.readNext()==0){
    tablereader.readRow(result_row);
    tableInformationList.emplace_back(std::make_tuple(result_row.at(0), result_row.at(1)));
  }
  tablereader.readEnd();

  return 0;
}

auto trustdble::getKeyStore(const char *shared_db,
                               const char *tablename,
                               KEY_STORE &table) -> int {
  std::string query = "'SELECT * FROM key_store WHERE tablename= ?'";
  std::vector<std::string> parameter_queries {"SET @1 = '"+ std::string(tablename) +"';"};

  std::vector<std::string> result_row;
  TableService tablereader;
  tablereader.query_prepared_stmt(query, std::string(shared_db), parameter_queries);
  tablereader.readInit();
  tablereader.readNext();
  tablereader.readRow(result_row);
  tablereader.readEnd();

  if (!result_row.empty()) {
    table.table_name = result_row.at(0);     //0
    table.encryption_key = result_row.at(1);  //1
    table.iv = result_row.at(2);              //2
    return 0;

  }
  return 1;
}

auto trustdble::getMetaDataForSharedDataTable(
        std::vector<std::tuple<std::string, std::string>> &tableInformationList,
    const char *shared_db) -> int {

  if (!shared_db) {
    return 1;
  }
  TableService tablereader;

  std::string query = "'SELECT partition_id, tableschema FROM '";

  std::vector<std::string> result_row;
  tablereader.query(query, std::string(shared_db), "shared_tables");
  tablereader.readInit();

  // read multiple_rows
  while(tablereader.readNext()==0){
    tablereader.readRow(result_row);
    tableInformationList.emplace_back(std::make_tuple(result_row.at(0), result_row.at(1)));
    result_row.clear();
  }
  tablereader.readEnd();

  return 0;
}

auto trustdble::getListOfKeys( std::vector<std::string> &list_of_keys,
                              const char *table_name, const char *shared_db)
    -> int {

  if (!shared_db) {
    return 1;
  }
  std::string query = "'SELECT partition_id FROM shared_tables where partition_id LIKE ?'";
  std::vector<std::string> parameter_queries {"SET @1 = '"+ std::string(table_name) +"';"};

  std::vector<std::string> result_row;
  TableService tablereader;
  tablereader.query_prepared_stmt(query, std::string(shared_db), parameter_queries);
  tablereader.readInit();

  // read multiple_rows
  while(tablereader.readNext()==0){
    tablereader.readRow(result_row);
    list_of_keys.emplace_back(result_row.at(0));
  }
  tablereader.readEnd();

  return 0;
}

auto trustdble::getInvite(const char *databasename,
                          SHARED_DATABASE &invite) -> int {

  if (!databasename) {
    return 1;
  }
  std::string query = "'SELECT * FROM invites where database_name = ?'";
  std::vector<std::string> parameter_queries {"SET @1 = '"+ std::string(databasename) +"';"};

  std::vector<std::string> result_row;
  TableService tablereader;
  tablereader.query_prepared_stmt(query, "trustdble", parameter_queries);
  tablereader.readInit();
  tablereader.readNext();
  tablereader.readRow(result_row);
  tablereader.readEnd();

  if (!result_row.empty()) {
    invite.name = result_row.at(0);               // 0
    invite.bc_type = result_row.at(1);            // 1
    invite.meta_address = result_row.at(2);       // 2
    invite.meta_chain_config = result_row.at(3);  // 3
    invite.encryption_key = result_row.at(4);     // 4
    invite.iv = result_row.at(5);                 // 5
    return 0;
  }
  return 1;
}

auto trustdble::deleteInvite(const char *databasename) -> int {

  if (!databasename) {
    return 1;
  }
  std::string query = "DELETE FROM trustdble.invites where database_name = '" + std::string(databasename)+"'";

  TableService tablereader;

  return tablereader.query(query);
}

auto trustdble::get_container_name(std::vector<std::string> &data_container_name,
                              const char *shared_db) -> int {
  if (!shared_db) {
    return 1;
  }  // error no database selected

  std::string query = "'SELECT container_name FROM '";

  TableService tablereader;
  tablereader.query(query, "trustdble", "docker_containers");
  tablereader.readInit();
  tablereader.readNext();
  tablereader.readRow(data_container_name);
  tablereader.readEnd();


  return 0;
}

auto trustdble::getNumShards(const std::string &databasename) -> int {
  DBUG_PRINT(LOG_TAG, ("table_service_method_call: getNumShards"));
  // error no database selected
  if (databasename.empty()) {
    DBUG_PRINT(LOG_TAG, ("getNumShards, databasename is empty"));
    // return -1;
  }
  TableService tablereader;

  std::string query = "'SELECT count(datachain_id) FROM '";

  std::vector<std::string> result_row;
  tablereader.query(query, databasename, META_TABLE_DATA_CHAINS_NAME);
  tablereader.readInit();
  tablereader.readNext();
  tablereader.readRow(result_row);
  tablereader.readEnd();

  int num_shards = stoi(result_row.at(0));

  // check num_shards is valid (>0)
  if (num_shards <= 0) {
    DBUG_PRINT(
        LOG_TAG,
        ("getNumShards, Invalid number of shards. It should be > 0! (%d)",
         num_shards));
    return -1;
  }
  return num_shards;
}

auto trustdble::copy_table (const char *shared_db, const char *table_name,
                       std::vector<std::vector<std::string>> &table_data) -> int {
  if (!shared_db) {return 1;} // error no database selected
  TableService tablereader;

  std::string query = "'SELECT * FROM '";


  tablereader.query(query, std::string(shared_db), std::string(table_name));
  tablereader.readInit();
  while(tablereader.readNext()==0){
    std::vector<std::string> result_row;
    tablereader.readRow(result_row);
    table_data.push_back(result_row);
  }
  tablereader.readEnd();
  return 0;
}

auto trustdble::get_hostname (std::string &hostname) -> int {
  std::string query = "'SELECT DISTINCT join_ip FROM '";
  std::vector<std::string> result_row;
  TableService tablereader;
  tablereader.query(query, "trustdble", "docker_containers");
  tablereader.readInit();
  tablereader.readNext();
  tablereader.readRow(result_row);
  tablereader.readEnd();
  hostname = result_row.empty() ? "localhost" : result_row.at(0);
  return 0;
}

auto trustdble::get_port (std::string &port) -> int {
  std::string query = "SELECT @@port";
  std::vector<std::string> result_row;
  TableService tablereader;
  tablereader.query(query);
  tablereader.readInit();
  tablereader.readNext();
  tablereader.readRow(result_row);
  tablereader.readEnd();
  port = result_row.at(0);
  return 0;
}

auto trustdble::get_local_table_schema (std::string &table_schema, const std::string &db_name,
                                            const std::string &table_name) -> int{
  if (!db_name.c_str()) {return 1;} // error no database selected

  std::string query = R"('SELECT CONCAT(column_name, \' \', column_type, \', \') as \'schema\' FROM COLUMNS WHERE TABLE_NAME= ? AND TABLE_SCHEMA= ? ORDER BY ORDINAL_POSITION')";
  std::vector<std::string> parameter_queries {"SET @1 = '"+ std::string(table_name) +"';", "SET @2 = '"+ std::string(db_name) +"';"};

  TableService tablereader;
  tablereader.query_prepared_stmt(query, "information_schema" , parameter_queries);
  tablereader.readInit();
  while (tablereader.readNext()==0) {
    std::vector<std::string> result_row;
    tablereader.readRow(result_row);
    table_schema.append(result_row.at(0));
  }
  // clear table schema (remove comma from the last position) {id int, val int[,]}
  if (!table_schema.empty()) {
    table_schema.pop_back();
    table_schema.pop_back();
    //table_schema.erase(std::prev(table_schema.end()));
  }

  tablereader.readEnd();

  return 0;
}

auto trustdble::get_list_of_local_tables (std::vector<std::string> &list_of_tables, const std::string &db_name) -> int{
  if (!db_name.c_str()) {return 1;} // error no database selected
  // check if table is LOCAL table (ENGINE='InnoDB')
  std::string query = R"('SELECT table_name FROM TABLES WHERE TABLE_SCHEMA=? AND TABLE_TYPE=\'BASE TABLE\' AND ENGINE=\'InnoDB\'')";
  std::vector<std::string> parameter_queries {"SET @1 = '"+ db_name +"';"};

  TableService tablereader;
  tablereader.query_prepared_stmt(query, "information_schema" , parameter_queries);
  tablereader.readInit();
  while (tablereader.readNext()==0) {
    std::vector<std::string> result_row;
    tablereader.readRow(result_row);
    // get table name
    std::string table_name = result_row.at(0);
    list_of_tables.push_back(table_name);
  }
  tablereader.readEnd();

  return 0;
}

auto trustdble::get_list_of_tables (std::vector<std::string> &list_of_tables, const char *shared_db) -> int {
  if (!shared_db) {return 1;} // error no database selected
  TableService tablereader;

  std::string query = "'SELECT DISTINCT SUBSTRING_INDEX(partition_id,'/', 1) AS name FROM '" ;

  tablereader.query(query,std::string(shared_db), SHARED_TABLES_NAME);
  tablereader.readInit();
  while(tablereader.readNext()==0){
    std::vector<std::string> result_row;
    tablereader.readRow(result_row);
    DBUG_PRINT(LOG_TAG, ("get_list_of_tables: table_name = %s", result_row.at(0).c_str()));
    list_of_tables.push_back(result_row.at(0));
  }
  tablereader.readEnd();

  return 0;
}

auto trustdble::GetDataNetworkConfig(std::vector<std::string> &data_chains_network_config,
    const char *shared_db) -> int{
  if (!shared_db) {
    return 1;
  }  // error no database selected
  TableService tablereader;

  std::string query = "'SELECT connection_information FROM '";

  tablereader.query(query, std::string(shared_db), META_TABLE_DATA_CHAINS_NAME);
  tablereader.readInit();
  tablereader.readNext();
  tablereader.readRow(data_chains_network_config);
  tablereader.readEnd();

  return 0;
}


auto trustdble::GetDataNetworkConfigForShard(std::string &data_chains_network_config,
                                             const std::string &database_name,
                                             const int shard_number) -> int{

  if (database_name.empty()) {return 1;} // error no database selected


  std::string query = "'SELECT * FROM " + META_TABLE_DATA_CHAINS_NAME+" where datachain_id= ?'";
  std::vector<std::string> parameter_queries {"SET @1 = "+ std::to_string(shard_number) +";"};

  TableService tablereader;
  std::vector<std::string> result_row;
  tablereader.query_prepared_stmt(query, database_name , parameter_queries);
  tablereader.readInit();
  tablereader.readNext();
  tablereader.readRow(result_row);
  tablereader.readEnd();

  if (!result_row.empty()) {
    data_chains_network_config = result_row.at(1);  // 1
    DBUG_PRINT(LOG_TAG, ("GetDataNetworkConfigForShard: shard_number = %d",
                         shard_number));
    DBUG_PRINT(LOG_TAG,
               ("GetDataNetworkConfigForShard: data_chains_network_config = %s",
                data_chains_network_config.c_str()));
    return 0;
  }

  return 1;
}

auto trustdble::get_table_schema(const std::string &table_name, std::string &table_schema,
                                   const char *shared_db) -> int{
  std::vector<std::tuple<std::string, std::string>> list; // list<0> is table_name, list<1> is table_schema
  if (getMetaDataForSharedDataTable(list, shared_db)!=0) {
    return 1;
  }
  int data_chain_id = 0;
  // get partition_id = table_name + '/' + data_chain_id
  const std::string partition_id = table_name + '/' + std::to_string(data_chain_id);
  for (auto entry:list) {
    if (partition_id.compare(std::get<0>(entry))==0) {
      table_schema = std::get<1>(entry);
      return 0;
    }
  }
  table_schema = "";
  return 0;
}

auto trustdble::getSchemaForSharedDataTable(const std::string &databasename,
                                                const std::string &tablename,
                                                SHARED_TABLE &table) -> int {

  if (databasename.empty() | tablename.empty()) {
    return 1;
  }  // error no database or table selected

  // get partition_id = table_name + '/' + data_chain_id
  const int shard_number = 0;
  const std::string partition_id = tablename + '/' + std::to_string(shard_number);

  std::string query = "'SELECT * FROM " + SHARED_TABLES_NAME + " where partition_id = ?'";
  std::vector<std::string> parameter_queries {"SET @1 = '"+ std::string(partition_id) +"';"};

  TableService tablereader;
  std::vector<std::string> result_row;
  tablereader.query_prepared_stmt(query, databasename , parameter_queries);
  tablereader.readInit();
  tablereader.readNext();
  tablereader.readRow(result_row);
  tablereader.readEnd();

  if (!result_row.empty()) {
    table.name = result_row.at(0);           // 0
    table.address = result_row.at(1);        // 1
    table.schema = result_row.at(2);         // 2
    return 0;
  }
  return 1;
}

auto trustdble::getAddressForSharedDataTable(
    const std::string &databasename, const std::string &tablename,
    const int shard_number, SHARED_TABLE &table) -> int {
    DBUG_PRINT(LOG_TAG,
             ("table_service_method_call: getAddressForSharedDataTable"));

  if (databasename.empty() | tablename.empty()) {
    return 1;
  }  // error no database or table selected
  TableService tablereader;

  const std::string partition_id = tablename + '/' + std::to_string(shard_number);

  std::string query = "'SELECT * FROM " + SHARED_TABLES_NAME + " where partition_id = ?'";
  std::vector<std::string> parameter_queries {"SET @1 = '"+ partition_id +"';"};
  std::vector<std::string> result_row;

  tablereader.query_prepared_stmt(query, databasename, parameter_queries);
  tablereader.readInit();
  tablereader.readNext();
  tablereader.readRow(result_row);
  tablereader.readEnd();

  if (!result_row.empty()) {
    table.name = result_row.at(0);     // 0
    table.address = result_row.at(1);  // 1
    table.schema = result_row.at(2);   // 2
    return 0;
  }
  return 1;
}
auto trustdble::getSgxMeasurement(SGX_MEASUREMENT &sgx_measurement) -> int {

  std::string query = "SELECT * FROM trustdble.enclave_measurement";

  std::vector<std::string> result_row;
  TableService tablereader;
  tablereader.query(query);
  tablereader.readInit();
  tablereader.readNext();
  tablereader.readRow(result_row);
  tablereader.readEnd();

  if (!result_row.empty()) {
    sgx_measurement.MRSIGNER = result_row.at(0);
    sgx_measurement.MRENCLAVE = result_row.at(1);
    sgx_measurement.isv_prod_id = result_row.at(2);
    sgx_measurement.isv_svn = result_row.at(3);
    return 0;
  }
  return 1;
}
auto trustdble::getDataContract(std::string name, DATA_CONTRACT &data_contract) -> int {

 std::string query = "SELECT * FROM trustdble.data_contracts where name='"+name+"'";
  std::vector<std::string> result_row;
  TableService tablereader;
  tablereader.query(query);
  tablereader.readInit();
  tablereader.readNext();
  tablereader.readRow(result_row);
  tablereader.readEnd();

  if (!result_row.empty()) {
    data_contract.name = result_row.at(0);
    data_contract.owner = result_row.at(1);
    data_contract.user = result_row.at(2);
    data_contract.logic = result_row.at(3);
    data_contract.parameters = result_row.at(4);
    return 0;
  }
  return 1;
}


auto trustdble::updateSharedDataTableAddress(
    const std::string &databasename, const SHARED_TABLE &table,
    int shard_number) -> int {
  DBUG_PRINT(LOG_TAG,
             ("table_service_method_call: updateSharedDataTableAddress"));
  if (databasename.empty()) {
    return 1;
  }  // error no database selected

  // get partition_id = table_name + '/' + data_chain_id
  const std::string partition_id = table.name + '/' + std::to_string(shard_number);

  DBUG_PRINT(LOG_TAG, ("table.partition_id = %s", partition_id.c_str()));
  DBUG_PRINT(LOG_TAG, ("table.address = %s", table.address.c_str()));
  DBUG_PRINT(LOG_TAG, ("table.schema = %s", table.schema.c_str()));

  std::string query =  "'update " + SHARED_TABLES_NAME + R"( set tableaddress= \')" + table.address +R"(\' where partition_id=?')";
  std::vector<std::string> parameter_queries {"SET @1 = '"+ std::string(partition_id) +"';"};

  TableService tablereader;
  tablereader.query_prepared_stmt(query, databasename , parameter_queries);

  return 0;
}

auto trustdble::updateSharedDatabase(const SHARED_DATABASE &database)
    -> int {
  DBUG_PRINT(LOG_TAG, ("table_service_method_call: updateSharedDatabase"));

  std::string query =
      R"('UPDATE shared_databases SET meta_table_address=\')" +
      database.meta_address + R"(\' where database_name=?')";
  std::vector<std::string> parameter_queries {"SET @1 = '"+ database.name +"';"};

  TableService tablereader;
  tablereader.query_prepared_stmt(query, "trustdble" , parameter_queries);
  return 0;
}

// update meta table
auto trustdble::updateSharedTable(const std::string &databasename,
                                      const SHARED_TABLE &table) -> int {
  DBUG_PRINT(LOG_TAG, ("table_service_method_call: updateSharedTable"));

  DBUG_PRINT(LOG_TAG, ("table.name = %s", table.name.c_str()));
  DBUG_PRINT(LOG_TAG, ("table.address = %s", table.address.c_str()));
  DBUG_PRINT(LOG_TAG, ("table.schema = %s", table.schema.c_str()));

  std::string query = "'INSERT INTO "  + META_TABLE_NAME +
                      R"( VALUES (?, \')" + table.address +
                      R"(\', ?)')";
  std::vector<std::string> parameter_queries {"SET @1 = '"+ table.name +"';", "SET @2 = '"+ table.schema +"';"};

  TableService tablereader;
  tablereader.query_prepared_stmt(query, databasename , parameter_queries);

  return 0;
}


auto TableService::query(const std::string &query, const std::string &database_name, const std::string &table_name) -> int {

  int error=0;
  init();

  m_con = mysql_init(NULL);
  if (m_con == NULL) {
    fprintf(stderr, "%s\n", mysql_error(m_con));
    return 1;
  }

  if(table_name == ""){
    if (mysql_real_connect(m_con, m_host.c_str(), m_username.c_str(),
                        m_password.c_str(), NULL, m_port, NULL,
                        0) == NULL) {
      fprintf(stderr, "%s\n", mysql_error(m_con));
      mysql_close(m_con);
      return 1;
    }
    error=mysql_real_query(m_con, query.c_str(), query.size());
  }
  else{
    if (mysql_real_connect(m_con, m_host.c_str(), m_username.c_str(),
                        m_password.c_str(), database_name.c_str(), m_port, NULL,
                        0) == NULL) {
      fprintf(stderr, "%s\n", mysql_error(m_con));
      mysql_close(m_con);
      return 1;
    }

    const std::string param_query="set @tablename ='" + table_name + "';";
    const std::string pre_query="set @queryToSelectStudentName := concat(" + query + ", @tablename);";
    const std::string prep_query="PREPARE executeQuery FROM @queryToSelectStudentName;";
    const std::string exec_query="execute executeQuery;";

    error=mysql_real_query(m_con, param_query.c_str(), param_query.size());
    error=mysql_real_query(m_con, pre_query.c_str(), pre_query.size());
    error=mysql_real_query(m_con, prep_query.c_str(), prep_query.size());
    error=mysql_real_query(m_con, exec_query.c_str(), exec_query.size());
  }

  m_result = mysql_store_result(m_con);
  if (m_result == NULL) {
    fprintf(stderr, "%s\n", mysql_error(m_con));
    mysql_close(m_con);
    return 2;
  }

  m_num_fields = mysql_num_fields(m_result);

  mysql_close(m_con);

  return error;
}

auto TableService::query_prepared_stmt(const std::string &query, const std::string &database_name, std::vector<std::string> parameter_queries) -> int {

  int error=0;
  init();

  m_con = mysql_init(NULL);
  if (m_con == NULL) {
    fprintf(stderr, "%s\n", mysql_error(m_con));
    return 1;
  }
  if (mysql_real_connect(m_con, m_host.c_str(), m_username.c_str(),
                         m_password.c_str(), database_name.c_str(), m_port, NULL,
                         0) == NULL) {
    fprintf(stderr, "%s\n", mysql_error(m_con));
    mysql_close(m_con);
    return 1;
  }

  const std::string prep_query="PREPARE stmt1 FROM " + query + ";";
  error=mysql_real_query(m_con, prep_query.c_str(), prep_query.size());

  std::string exec_query="EXECUTE stmt1 USING ";
  int param_index = 1;
  for(const auto& param_query: parameter_queries) {
    error=mysql_real_query(m_con, param_query.c_str(), param_query.size());
    if(param_index == 1)
      exec_query += "@"+std::to_string(param_index);
    else
      exec_query += ",@"+std::to_string(param_index);

    param_index++;
  }
  exec_query += ";";

  error=mysql_real_query(m_con, exec_query.c_str(), exec_query.size());

  m_result = mysql_store_result(m_con);
  if (m_result == NULL) {
    fprintf(stderr, "%s\n", mysql_error(m_con));
    mysql_close(m_con);
    return 1;
  }

  m_num_fields = mysql_num_fields(m_result);

  mysql_close(m_con);

  return error;
}
// Helper Methods
auto TableService::init() -> int {
  m_username = "root";
  m_password = "";
  m_host = "localhost";
  m_port = mysqld_port;
  return 0;
}

auto TableService::readInit() -> int {

  return 0;
}

auto TableService::readNext() -> int {
  if (m_result == NULL){
    m_row=NULL;
    return 1;
  }
  if ((m_row=mysql_fetch_row(m_result)))
    return 0;
  else
    return 1;
}

auto TableService::readEnd() -> int {
  if (m_result == NULL) return 1;
  mysql_free_result(m_result);
  return 0;
}

auto TableService::readRow(std::vector<std::string> &row) -> int {
  if(!m_row)
    return 0;
  for (size_t i = 0; i < m_num_fields; i++) {
    row.push_back(std::string(m_row[i]));
  }
  return 0;
}

auto TableService::escapeInput(const char *pStr) ->std::string {
    std::string result;
    while (*pStr) {
        if (strchr("\"'\r\n\t",*pStr))
        {
            //bad character, skip
        }
        else
        {
            result.push_back(*pStr);
        }
        ++pStr;
    }
    return result;
}
