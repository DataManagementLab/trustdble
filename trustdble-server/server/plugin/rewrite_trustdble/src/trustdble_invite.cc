#include "trustdble_invite.h"

#include <fstream>
#include <iostream>
#include <iomanip>


#ifdef RAPIDJSON_NO_SIZETYPEDEFINE
// if we build within the server, it will set RAPIDJSON_NO_SIZETYPEDEFINE
// globally and require to include my_rapidjson_size_t.h
#include "my_rapidjson_size_t.h"
#endif

#include "rapidjson/document.h"
#include "rapidjson/istreamwrapper.h"
#include "rapidjson/ostreamwrapper.h"
#include "rapidjson/writer.h"
#include <sstream>

using namespace rapidjson;

using namespace trustdble;

auto write_invite(const std::string &path, const SHARED_DATABASE &invite) -> bool{
  // Create json from invite
  Document jsonFile;
  jsonFile.SetObject();

  Value name;
  name = StringRef(invite.name.c_str(), invite.name.length());
  jsonFile.AddMember("database_name", name, jsonFile.GetAllocator());
  Value address;
  address = StringRef(invite.meta_address.c_str(), invite.meta_address.size());
  jsonFile.AddMember("meta_table_address", address, jsonFile.GetAllocator());

  Document metaChainConfig_document;
  metaChainConfig_document.Parse(invite.meta_chain_config.c_str());
  jsonFile.AddMember("meta_chain_config", metaChainConfig_document, jsonFile.GetAllocator());
  Value bcType;
  bcType = StringRef(invite.bc_type.c_str(), invite.bc_type.size());
  jsonFile.AddMember("blockchain_type", bcType, jsonFile.GetAllocator());
  Value encryptionKey;
  encryptionKey = StringRef(invite.encryption_key.c_str(), invite.encryption_key.size());
  jsonFile.AddMember("encryption_key", encryptionKey, jsonFile.GetAllocator());
  Value iv;
  iv = StringRef(invite.iv.c_str(), invite.iv.size());
  jsonFile.AddMember("iv", iv, jsonFile.GetAllocator());

  // Write json to file
  std::ofstream ofs(path+""+invite.name+".json");
  OStreamWrapper osw(ofs);

  Writer<OStreamWrapper> writer(osw);
  jsonFile.Accept(writer);

  return true;
}

auto read_invite_file(const std::string &path, SHARED_DATABASE &invite) -> bool{
  // Read json file
  std::ifstream ifs(path);
  std::stringstream buffer;
  buffer<<ifs.rdbuf();
  const std::string jsonStr  = buffer.str();
  return parse_invite(jsonStr.c_str(), invite);
}

auto parse_invite(const char *invite_key, SHARED_DATABASE &invite) -> bool{
  // Read json file
  Document jsonFile;
  ParseResult ok = jsonFile.Parse(invite_key);
  if(!ok){
    return false;
  }
  // Put json to invite
  invite.name=jsonFile["database_name"].GetString();
  invite.meta_address=jsonFile["meta_table_address"].GetString();


  // Use buffer to write the subdocument
  StringBuffer buffer;
  Writer<StringBuffer> writer(buffer);

  Value &subDocument = jsonFile["meta_chain_config"];
  subDocument.Accept(writer);
  invite.meta_chain_config = buffer.GetString();

  buffer.Clear();
  writer.Reset(buffer);

  invite.bc_type=jsonFile["blockchain_type"].GetString();
  invite.encryption_key=jsonFile["encryption_key"].GetString();
  invite.iv=jsonFile["iv"].GetString();

  return true;
}

auto parse_encrypted_invite(const char *encrypted_invite, ENCRYPTED_STRUCT &invite) -> bool{
  // Read json file
  Document jsonFile;
  ParseResult ok = jsonFile.Parse(encrypted_invite);
  if(!ok){
    return false;
  }
  //check wether all members are initialized
  if(!jsonFile.HasMember("key") || !jsonFile.HasMember("iv")|| !jsonFile.HasMember("invite"))
    return false;
  // Put json to invite
  invite.encrypted_key=jsonFile["key"].GetString();
  invite.encrytpted_iv=jsonFile["iv"].GetString();
  invite.encrypted_string=jsonFile["invite"].GetString();

  return true;
}

auto parse_table_encrypted_invite(const char *encrypted_invite, std::string &invite) -> bool{
  // Read json file
  Document jsonFile;
  ParseResult ok = jsonFile.Parse(encrypted_invite);
  if(!ok){
    return false;
  }
  //check wether member is initialized
  if(!jsonFile.HasMember("invite"))
    return false;
  // Put json to invite
  invite=jsonFile["invite"].GetString();

  return true;
}

auto parse_table_invite(const char *invite_key, KEY_STORE &invite) -> bool{
  // Read json file
  Document jsonFile;
  ParseResult ok = jsonFile.Parse(invite_key);
  if(!ok){
    return false;
  }
  //check wether all members are initialized
  if(!jsonFile.HasMember("db_name") ||!jsonFile.HasMember("table_name") || !jsonFile.HasMember("encryption_key")|| !jsonFile.HasMember("iv"))
    return false;
  // Put json to invite
  invite.db_name=jsonFile["db_name"].GetString();
  invite.table_name=jsonFile["table_name"].GetString();
  invite.encryption_key=jsonFile["encryption_key"].GetString();
  invite.iv=jsonFile["iv"].GetString();

  return true;
}
auto parse_encrypted_data_contract(const char *encrypted_data_contract, ENCRYPTED_STRUCT &data_contract) -> bool{
  // Read json file
  Document jsonFile;
  ParseResult ok = jsonFile.Parse(encrypted_data_contract);
  if(!ok){
    return false;
  }
  //check wether all members are initialized
  if(!jsonFile.HasMember("key") || !jsonFile.HasMember("iv")|| !jsonFile.HasMember("data_contract"))
    return false;
  // Put json to data contract
  data_contract.encrypted_key=jsonFile["key"].GetString();
  data_contract.encrytpted_iv=jsonFile["iv"].GetString();
  data_contract.encrypted_string=jsonFile["data_contract"].GetString();

  return true;
}

auto parse_data_contract(const char *data_contract_key, REMOTE_DATA_CONTRACT &data_contract) -> bool{
  // Read json file
  Document jsonFile;
  ParseResult ok = jsonFile.Parse(data_contract_key);
  if(!ok){
    return false;
  }
  //check wether all members are initialized
  if(!jsonFile.HasMember("name") ||!jsonFile.HasMember("owner") || !jsonFile.HasMember("user")|| !jsonFile.HasMember("logic")
     || !jsonFile.HasMember("parameters") || !jsonFile.HasMember("server_address"))
    return false;
  // Put json to data contract
  data_contract.name=jsonFile["name"].GetString();
  data_contract.owner=jsonFile["owner"].GetString();
  data_contract.user=jsonFile["user"].GetString();
  data_contract.logic=jsonFile["logic"].GetString();
  data_contract.parameters=jsonFile["parameters"].GetString();
  data_contract.server_address=jsonFile["server_address"].GetString();
  data_contract.MRSIGNER=jsonFile["MRSIGNER"].GetString();
  data_contract.MRENCLAVE=jsonFile["MRENCLAVE"].GetString();
  data_contract.isv_prod_id=jsonFile["isv_prod_id"].GetString(); 
  data_contract.isv_svn=jsonFile["isv_svn"].GetString();

  return true;
}

auto parse_local_table_invite(const char *invite_key, FEDERATED_TABLE &invite) -> bool{
  // Read json file
  Document jsonFile;
  ParseResult ok = jsonFile.Parse(invite_key);
  if(!ok){
    return false;
  }
  // Put json to invite
  invite.db_name=jsonFile["db_name"].GetString();
  invite.table_name=jsonFile["table_name"].GetString();
  invite.table_schema=jsonFile["table_schema"].GetString();
  invite.hostname=jsonFile["hostname"].GetString();
  invite.port=jsonFile["port"].GetString();
  invite.user_name=jsonFile["user_name"].GetString();
  invite.password=jsonFile["password"].GetString();

  return true;
}
