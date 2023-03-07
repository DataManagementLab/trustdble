#ifndef PLUGIN_TRUSTDBLE_REWRITER
#define PLUGIN_TRUSTDBLE_REWRITER

#include <string>

struct REWRITE_QUERY_TUPLE {
  std::string SHARED;
  std::string UNSHARED;
};

const REWRITE_QUERY_TUPLE CREATE_DATABASE = { "create shared database ", "create database " };
const REWRITE_QUERY_TUPLE LOAD_DATABASE = { "load shared database ", "create database " };
const REWRITE_QUERY_TUPLE DROP_DATABASE = { "drop shared database ", "drop database " };
const REWRITE_QUERY_TUPLE CREATE_TABLE = { "create shared table ", "create table " };
const REWRITE_QUERY_TUPLE LOAD_TABLE = { "load shared table ", "" };
const REWRITE_QUERY_TUPLE DROP_TABLE = { "drop shared table ", "drop table " };
const REWRITE_QUERY_TUPLE LOAD_TABLES = { "load shared tables", "" };
const REWRITE_QUERY_TUPLE LOAD_SHARDS = { "load shards from ", "" };
const REWRITE_QUERY_TUPLE REPARTITION_DATABASE = { "repartition shared database ", "" };
const REWRITE_QUERY_TUPLE PARTITION_TABLES = { "partition tables ", "" };
const REWRITE_QUERY_TUPLE CREATE_INVITE = { "create database invite ", "" };
const REWRITE_QUERY_TUPLE ADD_INVITE = { "add database invite ", "" };
const REWRITE_QUERY_TUPLE CREATE_TABLE_INVITE = { "create table invite ", "" };
const REWRITE_QUERY_TUPLE ADD_TABLE_INVITE = { "add table invite ", "" };
const REWRITE_QUERY_TUPLE ADD_DATA_CONTRACT = { "add data contract ", "" };
const REWRITE_QUERY_TUPLE CREATE_DATA_CONTRACT = { "create data contract ", "" };
const REWRITE_QUERY_TUPLE ENCRYPT_DATA_CONTRACT = { "encrypt data contract ", "" };

/**
 * @brief Rewrites a TrustDBle shared statement query into a mysql statement query
 *
 * @param query Query to be rewritten
 * @param rewrite_rule TrustDBle rewrite-rule how to rewrite the query
 */
void rewrite_query(std::string &query, REWRITE_QUERY_TUPLE rewrite_rule);

/**
 * @brief Creates a config file for the given blockchain type including manager and adapter configs
 *
 * @param config_path Path of config file including name
 * @param bctype blockchain type
 * @param number Number of database
 * @param dependency_dir Path of dependency folder containing bcManager and bcAdapter
 */
void generateConfig(const std::string &config_path, const std::string &bctype, const std::string &dependency_dir);

#endif // PLUGIN_TRUSTDBLE_REWRITER
