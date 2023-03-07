/*  Copyright (c) 2015, 2020, Oracle and/or its affiliates. All rights reserved.

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License, version 2.0,
    as published by the Free Software Foundation.

    This program is also distributed with certain software (including
    but not limited to OpenSSL) that is licensed under separate terms,
    as designated in a particular file or component or in included license
    documentation.  The authors of MySQL hereby grant you an additional
    permission to link the program and your derivative works with the
    separately licensed software that they have included with MySQL.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License, version 2.0, for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301  USA */

#include <ctype.h>
#include <mysql/plugin.h>
#include <mysql/plugin_audit.h>
#include <mysql/psi/mysql_memory.h>
#include <mysql/service_mysql_alloc.h>
#include <string.h>
#include "my_dbug.h"
#include <iomanip>
#include <iostream>

#include "trustdble_command.h"

using namespace blockchain_manager;

/* instrument the memory allocation */
#ifdef HAVE_PSI_INTERFACE
static PSI_memory_key key_memory_rewrite_trustdble;

static PSI_memory_info all_rewrite_memory[] = {
    {&key_memory_rewrite_trustdble, "rewrite_trustdble", 0, 0, PSI_DOCUMENT_ME}};

#else
#define plugin_init NULL
#define key_memory_rewrite_trustdble PSI_NOT_INSTRUMENTED
#endif /* HAVE_PSI_INTERFACE */

static SERVICE_TYPE(registry) *reg_srv = nullptr;
SERVICE_TYPE(log_builtins) *log_bi = nullptr;
SERVICE_TYPE(log_builtins_string) *log_bs = nullptr;

// plugin init
static int plugin_init(MYSQL_PLUGIN) {
  const char *category = "sql";
  int count;

  count = static_cast<int>(array_elements(all_rewrite_memory));
  mysql_memory_register(category, all_rewrite_memory, count);

  // Initialize error logging service.
  if (init_logging_service_for_plugin(&reg_srv, &log_bi, &log_bs)) return 1;

  return 0; /* success */
}

// System variables for configuration
static char* config_dependency_path;
static char* config_configuration_path;
static char* config_invite_path;
static char* config_server_number;

static MYSQL_SYSVAR_STR(bc_dependency_path, config_dependency_path, PLUGIN_VAR_RQCMDARG | PLUGIN_VAR_READONLY,
                        "Path to dependency folder", nullptr, nullptr,
                        nullptr);
static MYSQL_SYSVAR_STR(bc_configuration_path, config_configuration_path, PLUGIN_VAR_RQCMDARG | PLUGIN_VAR_READONLY,
                        "Path to BlockchainManager and BlockchainAdapter configuration", nullptr, nullptr,
                        nullptr);
static MYSQL_SYSVAR_STR(invite_path, config_invite_path, PLUGIN_VAR_RQCMDARG | PLUGIN_VAR_READONLY,
                        "Path to invite folder", nullptr, nullptr,
                        nullptr);
static MYSQL_SYSVAR_STR(server_number, config_server_number, PLUGIN_VAR_RQCMDARG | PLUGIN_VAR_READONLY,
                        "Server number", nullptr, nullptr,
                        nullptr);

static SYS_VAR *rewrite_system_variables[] = {
    MYSQL_SYSVAR(bc_dependency_path), // dependency folder path
    MYSQL_SYSVAR(bc_configuration_path), // configuration folder path
    MYSQL_SYSVAR(invite_path), // path for invite folder
    MYSQL_SYSVAR(server_number), // server number
    nullptr
};

static int rewrite_trustdble(MYSQL_THD thd, mysql_event_class_t event_class,
                        const void *event) {
  if (event_class == MYSQL_AUDIT_PARSE_CLASS) {
    const struct mysql_event_parse *event_parse =
        static_cast<const struct mysql_event_parse *>(event);
    DBUG_PRINT(LOG_TAG, ("Rewrite-Plugin: Input-query: %s", event_parse->query.str));
    if (event_parse->event_subclass == MYSQL_AUDIT_PARSE_PREPARSE) {
      // Set lex input string
      Lex_input_stream *lip = &thd->m_parser_state->m_lip;

      // Prepare string
      std::string orig_query = event_parse->query.str;

      // Lower all characters
      // Disabled because path is part of the statement load shared database and it is case sensitive
      //std::transform(orig_query.begin(), orig_query.end(), orig_query.begin(), [](unsigned char c) { return std::tolower(c); });

      // clean parameter string from multiple white spaces
      tidy_spaces(orig_query);
      DBUG_PRINT(LOG_TAG, ("Rewrite-Plugin: orig_query :: %s", orig_query.c_str()));

      std::string new_query = orig_query;
      std::string first_query = orig_query;

      size_t pos = orig_query.find_first_of(';');
      if (pos!=std::string::npos) {
        first_query=orig_query.substr(0,pos);
      }
      // config paremeters needed for the execute method
      CONFIG_SET config={config_dependency_path, config_configuration_path, config_invite_path, config_server_number};
      std::string first_query_lowercase = first_query;
      boost::algorithm::to_lower(first_query_lowercase);
      if ( first_query_lowercase.find(CREATE_DATABASE.SHARED) == 0 ) {
        // extract values of parameters
        std::string parameters = first_query.substr(CREATE_DATABASE.SHARED.length(), first_query.length());
        std::map<std::string,PARSE_PARAMETER> parameter_set;
        //setting the parameter set for the specific command
        parameter_set["database_name"]={ ParameterType::positional,"0", "", ""};
        parameter_set["type"]={ ParameterType::keyvalue,"type", "", ""};
        parameter_set["shards"]={ ParameterType::keyvalue,"shards", "1", ""};
        parameter_set["encryption"]={ ParameterType::keyvalue,"encrypt", "true", ""};

        std::unique_ptr<TrustdbleCommand> command = std::make_unique<TrustdbleCommand>(TrustdbleSharedCommand::create_database,parameters,parameter_set,thd);
        if(command!=nullptr){
        command->parse(); //if parsing fails return error code
        command->execute(new_query,config);
        }

        *((int *)event_parse->flags) |=
            (int)MYSQL_AUDIT_PARSE_REWRITE_PLUGIN_QUERY_REWRITTEN;
      }
      else if ( first_query_lowercase.find(REPARTITION_DATABASE.SHARED) == 0 ) {
        // extract values of parameters
        std::string parameters = first_query.substr(REPARTITION_DATABASE.SHARED.length(), first_query.length());
        std::map<std::string,PARSE_PARAMETER> parameter_set;
        // setting the parameter set for the specific command
        // parse database name
        parameter_set["database_name"] = {ParameterType::positional, "0", "", ""};
        // parse number of shards
        parameter_set["shards"] = {ParameterType::keyvalue, "shards", "", ""};

        std::unique_ptr<TrustdbleCommand> command = std::make_unique<TrustdbleCommand>(TrustdbleSharedCommand::repartition_database,parameters,parameter_set,thd);
        if (command != nullptr) {
          command->parse(); // if parsing fails return error code
          command->execute(new_query, config);
        }

        *((int *)event_parse->flags) |=
            (int)MYSQL_AUDIT_PARSE_REWRITE_PLUGIN_QUERY_REWRITTEN;
      }
      else if ( first_query_lowercase.find(PARTITION_TABLES.SHARED) == 0 ) {
        // extract values of parameters
        std::string parameters = first_query.substr(PARTITION_TABLES.SHARED.length(), first_query.length());
        std::map<std::string,PARSE_PARAMETER> parameter_set;
        // setting the parameter set for the specific command
        parameter_set["database_name"] = { ParameterType::positional,"0", "", ""};
        parameter_set["shards"] = { ParameterType::keyvalue,"shards", "", ""};

        std::unique_ptr<TrustdbleCommand> command = std::make_unique<TrustdbleCommand>(TrustdbleSharedCommand::partition_tables,parameters,parameter_set,thd);
        if (command!=nullptr) {
        command->parse(); // if parsing fails return error code
        command->execute(new_query, config);
        }
        *((int *)event_parse->flags) |=
            (int)MYSQL_AUDIT_PARSE_REWRITE_PLUGIN_QUERY_REWRITTEN;
      }
      else if ( first_query_lowercase.find(DROP_DATABASE.SHARED) == 0 ) {
        // extract values of parameters
        std::string parameters = first_query.substr(DROP_DATABASE.SHARED.length(), first_query.length());
        std::map<std::string,PARSE_PARAMETER> parameter_set;
        //setting the parameter set for the specific command
        parameter_set["database_name"]={ ParameterType::positional,"0", "", ""};

        std::unique_ptr<TrustdbleCommand> command = std::make_unique<TrustdbleCommand>(TrustdbleSharedCommand::drop_database,parameters,parameter_set,thd);
        if(command!=nullptr){
        command->parse(); //if parsing fails return error code
        command->execute(new_query,config);
        }

        *((int *)event_parse->flags) |=
            (int)MYSQL_AUDIT_PARSE_REWRITE_PLUGIN_QUERY_REWRITTEN;
      }
      else if ( first_query_lowercase.find(LOAD_DATABASE.SHARED) == 0 ) {
        // extract values of parameters
        std::string parameters = first_query.substr(LOAD_DATABASE.SHARED.length(), first_query.length());
        std::map<std::string,PARSE_PARAMETER> parameter_set;
        //setting the parameter set for the specific command
        parameter_set["database_name"]={ ParameterType::positional,"0", "", ""};

        std::unique_ptr<TrustdbleCommand> command = std::make_unique<TrustdbleCommand>(TrustdbleSharedCommand::load_database,parameters,parameter_set,thd);
        if(command!=nullptr){
          command->parse();
          command->execute(new_query,config);
        }

        *((int *)event_parse->flags) |=
            (int)MYSQL_AUDIT_PARSE_REWRITE_PLUGIN_QUERY_REWRITTEN;

      }
      else if ( first_query_lowercase.find(LOAD_SHARDS.SHARED) == 0 ) {
        // extract values of parameters
        std::string parameters = first_query.substr(LOAD_SHARDS.SHARED.length(), first_query.length());
        std::map<std::string,PARSE_PARAMETER> parameter_set;
        //setting the parameter set for the specific command
        parameter_set["database_name"]={ ParameterType::positional,"0", "", ""};

        std::unique_ptr<TrustdbleCommand> command = std::make_unique<TrustdbleCommand>(TrustdbleSharedCommand::load_shards,parameters,parameter_set,thd);
        if(command!=nullptr){
        command->parse(); //if parsing fails return error code
        command->execute(new_query,config);
        }
        *((int *)event_parse->flags) |=
            (int)MYSQL_AUDIT_PARSE_REWRITE_PLUGIN_QUERY_REWRITTEN;
        }
      else if ( first_query_lowercase.find(CREATE_TABLE.SHARED) == 0 ) {
        // extract values of parameters
        std::string parameters = first_query.substr(CREATE_TABLE.SHARED.length(), first_query.length());
        std::map<std::string,PARSE_PARAMETER> parameter_set;
        //setting the parameter set for the specific command
        parameter_set["table_name"]={ ParameterType::positional,"0", "", ""};
        parameter_set["schema"]={ ParameterType::positional,"1", "", ""};

        std::unique_ptr<TrustdbleCommand> command = std::make_unique<TrustdbleCommand>(TrustdbleSharedCommand::create_table,parameters,parameter_set,thd);
        if(command!=nullptr){
        command->parse(); //if parsing fails return error code
        command->execute(new_query,config);
        }
        *((int *)event_parse->flags) |=
            (int)MYSQL_AUDIT_PARSE_REWRITE_PLUGIN_QUERY_REWRITTEN;
        }
      else if ( first_query_lowercase.find(DROP_TABLE.SHARED) == 0 ) {
        // extract values of parameters
        std::string parameters = first_query.substr(DROP_TABLE.SHARED.length(), first_query.length());
        std::map<std::string,PARSE_PARAMETER> parameter_set;
        //setting the parameter set for the specific command
        parameter_set["table_name"]={ ParameterType::positional,"0", "", ""};

        std::unique_ptr<TrustdbleCommand> command = std::make_unique<TrustdbleCommand>(TrustdbleSharedCommand::drop_table,parameters,parameter_set,thd);
        if(command!=nullptr){
        command->parse(); //if parsing fails return error code
        command->execute(new_query,config);
        }
        *((int *)event_parse->flags) |=
            (int)MYSQL_AUDIT_PARSE_REWRITE_PLUGIN_QUERY_REWRITTEN;
        }
      else if ( first_query_lowercase.find(LOAD_TABLE.SHARED) == 0 ) {
        // extract values of parameters
        std::string parameters = first_query.substr(LOAD_TABLE.SHARED.length(), first_query.length());
        std::map<std::string,PARSE_PARAMETER> parameter_set;
        //setting the parameter set for the specific command
        parameter_set["table_name"]={ ParameterType::positional,"0", "", ""};

        std::unique_ptr<TrustdbleCommand> command = std::make_unique<TrustdbleCommand>(TrustdbleSharedCommand::load_table,parameters,parameter_set,thd);
        if(command!=nullptr){
        command->parse(); //if parsing fails return error code
        command->execute(new_query,config);
        }
        *((int *)event_parse->flags) |=
            (int)MYSQL_AUDIT_PARSE_REWRITE_PLUGIN_QUERY_REWRITTEN;
        }
      else if ( first_query_lowercase.find(LOAD_TABLES.SHARED) == 0 ) {
        // extract values of parameters
        std::string parameters = first_query.substr(LOAD_TABLE.SHARED.length(), first_query.length());
        std::map<std::string,PARSE_PARAMETER> parameter_set;

        std::unique_ptr<TrustdbleCommand> command = std::make_unique<TrustdbleCommand>(TrustdbleSharedCommand::load_tables, parameters,parameter_set,thd);
        if(command!=nullptr){
        command->execute(new_query,config);
        }
        *((int *)event_parse->flags) |=
            (int)MYSQL_AUDIT_PARSE_REWRITE_PLUGIN_QUERY_REWRITTEN;
      }
      else if ( first_query_lowercase.find(CREATE_INVITE.SHARED) == 0 ) {
        // extract values of parameters
        std::string parameters = first_query.substr(CREATE_INVITE.SHARED.length(), first_query.length());
        std::map<std::string,PARSE_PARAMETER> parameter_set;

        parameter_set["database_name"]={ ParameterType::positional,"0", "", ""};
        parameter_set["public_key"]={ ParameterType::keyvalue,"public_key", "", ""};
        std::unique_ptr<TrustdbleCommand> command = std::make_unique<TrustdbleCommand>(TrustdbleSharedCommand::create_invite, parameters,parameter_set,thd);
        if(command!=nullptr){
          command->parse();
          command->execute(new_query,config);
        }
        *((int *)event_parse->flags) |=
            (int)MYSQL_AUDIT_PARSE_REWRITE_PLUGIN_QUERY_REWRITTEN;
      }
      else if ( first_query_lowercase.find(ADD_INVITE.SHARED) == 0 ) {
        // extract values of parameters
        std::string parameters = first_query.substr(ADD_INVITE.SHARED.length(), first_query.length());
        std::map<std::string,PARSE_PARAMETER> parameter_set;

        parameter_set["encrypted_invite"]={ ParameterType::keyvalue,"encrypted_invite", "", ""};
        std::unique_ptr<TrustdbleCommand> command = std::make_unique<TrustdbleCommand>(TrustdbleSharedCommand::add_invite, parameters,parameter_set,thd);

        if(command!=nullptr){
          command->parse();
          command->execute(new_query,config);
        }
        *((int *)event_parse->flags) |=
            (int)MYSQL_AUDIT_PARSE_REWRITE_PLUGIN_QUERY_REWRITTEN;
      }
      else if ( first_query_lowercase.find(CREATE_TABLE_INVITE.SHARED) == 0 ) {
        // extract values of parameters
        std::string parameters = first_query.substr(CREATE_TABLE_INVITE.SHARED.length(), first_query.length());
        std::map<std::string,PARSE_PARAMETER> parameter_set;

        parameter_set["table_name"]={ ParameterType::positional,"0", "", ""};
        parameter_set["public_key"]={ ParameterType::keyvalue,"public_key", "", ""};
        std::unique_ptr<TrustdbleCommand> command = std::make_unique<TrustdbleCommand>(TrustdbleSharedCommand::create_table_invite, parameters,parameter_set,thd);
        if(command!=nullptr){
          command->parse();
          command->execute(new_query,config);
        }
        *((int *)event_parse->flags) |=
            (int)MYSQL_AUDIT_PARSE_REWRITE_PLUGIN_QUERY_REWRITTEN;
      }
      else if ( first_query_lowercase.find(ADD_TABLE_INVITE.SHARED) == 0 ) {
        // extract values of parameters
        std::string parameters = first_query.substr(ADD_TABLE_INVITE.SHARED.length(), first_query.length());
        std::map<std::string,PARSE_PARAMETER> parameter_set;

        parameter_set["encrypted_invite"]={ ParameterType::keyvalue,"encrypted_invite", "", ""};
        std::unique_ptr<TrustdbleCommand> command = std::make_unique<TrustdbleCommand>(TrustdbleSharedCommand::add_table_invite, parameters,parameter_set,thd);
        if(command!=nullptr){
          command->parse();
          command->execute(new_query,config);
        }
        *((int *)event_parse->flags) |=
            (int)MYSQL_AUDIT_PARSE_REWRITE_PLUGIN_QUERY_REWRITTEN;
      }
      else if ( first_query_lowercase.find(ADD_DATA_CONTRACT.SHARED) == 0 ) {
        // extract values of parameters
        std::string parameters = first_query.substr(ADD_DATA_CONTRACT.SHARED.length(), first_query.length());
        std::map<std::string,PARSE_PARAMETER> parameter_set;

        parameter_set["encrypted_data_contract"]={ ParameterType::keyvalue,"encrypted_data_contract", "", ""};
        std::unique_ptr<TrustdbleCommand> command = std::make_unique<TrustdbleCommand>(TrustdbleSharedCommand::add_data_contract, parameters,parameter_set,thd);
        if(command!=nullptr){
          command->parse();
          command->execute(new_query,config);
        }
        *((int *)event_parse->flags) |=
            (int)MYSQL_AUDIT_PARSE_REWRITE_PLUGIN_QUERY_REWRITTEN;
      }
      else if ( first_query_lowercase.find(CREATE_DATA_CONTRACT.SHARED) == 0 ) {
        // extract values of parameters
        std::string parameters = first_query.substr(CREATE_DATA_CONTRACT.SHARED.length(), first_query.length());
        std::map<std::string,PARSE_PARAMETER> parameter_set;

        parameter_set["name"]={ ParameterType::keyvalue,"name", "", ""};
        parameter_set["owner"]={ ParameterType::keyvalue,"owner", "", ""};
        parameter_set["user"]={ ParameterType::keyvalue,"user", "", ""};
        parameter_set["logic_path"]={ ParameterType::keyvalue,"logic_path", "", ""};
        parameter_set["parameters"]={ ParameterType::keyvalue,"parameters", "", ""};
        std::unique_ptr<TrustdbleCommand> command = std::make_unique<TrustdbleCommand>(TrustdbleSharedCommand::create_data_contract, parameters,parameter_set,thd);
        if(command!=nullptr){
          command->parse();
          command->execute(new_query,config);
        }
        *((int *)event_parse->flags) |=
            (int)MYSQL_AUDIT_PARSE_REWRITE_PLUGIN_QUERY_REWRITTEN;
      }
       else if ( first_query_lowercase.find(ENCRYPT_DATA_CONTRACT.SHARED) == 0 ) {
        // extract values of parameters
        std::string parameters = first_query.substr(ENCRYPT_DATA_CONTRACT.SHARED.length(), first_query.length());
        std::map<std::string,PARSE_PARAMETER> parameter_set;

        parameter_set["name"]={ ParameterType::keyvalue,"name", "", ""};
        parameter_set["public_key"]={ ParameterType::keyvalue,"public_key", "", ""};
        std::unique_ptr<TrustdbleCommand> command = std::make_unique<TrustdbleCommand>(TrustdbleSharedCommand::encrypt_data_contract, parameters,parameter_set,thd);
        if(command!=nullptr){
          command->parse();
          command->execute(new_query,config);
        }
        *((int *)event_parse->flags) |=
            (int)MYSQL_AUDIT_PARSE_REWRITE_PLUGIN_QUERY_REWRITTEN;
      }
      else {
        if (lip->get_ptr() > lip->get_end_of_query()
          || orig_query.find("trustdble.shared_databases") != orig_query.npos
          || orig_query.find("meta_table") != orig_query.npos
          || orig_query.find("shared_tables") != orig_query.npos
          || orig_query.find("data_chains") != orig_query.npos
          || orig_query.find("trustdble.docker_containers") != orig_query.npos){
          // The original end of the query is smaller than the current length: This means we have previously rewritten the query and we now need to update its lenght, otherwise the mysql parser will throw an error.
          // All our rewrittende query endet with update/create/delete our system table.
          // After the summary, when orig_query contains the system table name, we think it is a rewritten query
          // Otherwise it is not
          if(orig_query.find("?") != orig_query.npos){
            return 0;
          }
          new_query = orig_query + ";";
          *((int *)event_parse->flags) |=
              (int)MYSQL_AUDIT_PARSE_REWRITE_PLUGIN_QUERY_REWRITTEN;

        }
        else {
          return 0;
        }
      }
      DBUG_PRINT(LOG_TAG, ("Rewrite-Plugin: Rewritten-query: %s", new_query.c_str()));
      char *res_string = static_cast<char *>(my_malloc(key_memory_rewrite_trustdble, new_query.length() + 1, MYF(0)));
      memcpy(res_string, new_query.c_str(), new_query.length());
      event_parse->rewritten_query->str = res_string;
      event_parse->rewritten_query->length = new_query.length();
    }
  }

  return 0;
}

/* Audit plugin descriptor */
static struct st_mysql_audit rewrite_trustdble_descriptor = {
    MYSQL_AUDIT_INTERFACE_VERSION, /* interface version */
    nullptr,                       /* release_thd()     */
    rewrite_trustdble,                 /* event_notify()    */
    {
        0,
        0,
        (unsigned long)MYSQL_AUDIT_PARSE_ALL,
    } /* class mask        */
};

/* Plugin descriptor */
mysql_declare_plugin(rewrite_trustdble) {
    MYSQL_AUDIT_PLUGIN,          /* plugin type                   */
    &rewrite_trustdble_descriptor, /* type specific descriptor      */
    "rewrite_trustdble",           /* plugin name                   */
    PLUGIN_AUTHOR_ORACLE,        /* author                        */
    "An example of a query rewrite"
    " plugin that rewrites all queries"
    " to lower case",   /* description                   */
    PLUGIN_LICENSE_GPL, /* license                       */
    plugin_init,        /* plugin initializer            */
    nullptr,            /* plugin check uninstall        */
    nullptr,            /* plugin deinitializer          */
    0x0002,             /* version                       */
    nullptr,            /* status variables              */
    rewrite_system_variables,            /* system variables              */
    nullptr,            /* reserverd                     */
    0                   /* flags                         */
} mysql_declare_plugin_end;
