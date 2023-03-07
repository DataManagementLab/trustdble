#ifndef TRUSTDBLE_INVITE_H
#define TRUSTDBLE_INVITE_H

#include <string>
#include "blockchain/table_service.h"

using namespace trustdble;
/**
 * @brief Stores a TrustDble invite into to a file.
 *
 * @param path Path where to store the invite file
 * @param invite The invite containing all meta data
 *
 * @return true when successfull otherwise false
 *
 */
auto write_invite(const std::string &path, const SHARED_DATABASE &invite) -> bool;

/**
 * @brief Reads a TrustDBle invite from a file in a TrustDBle invite struct.
 *
 * @param path Path of the invite file
 * @param invite TrustDBle invite struct for holding the data from the file
 *
 * @return true when successfull otherwise false
 */
auto read_invite_file(const std::string &path, SHARED_DATABASE &invite) -> bool;

/**
 * @brief Reads a TrustDBle invite from a json formatted string into a TrustDBle invite struct.
 *
 * @param invite_key json formatted string containing invite information
 * @param invite TrustDBle invite struct for holding the data from the file
 *
 * @return true when successfull otherwise false
 */
auto parse_invite(const char *invite_key, SHARED_DATABASE &invite) -> bool;

/**
 * @brief Reads a TrustDBle encrypted invite from a json formatted string into a TrustDBle encrypted invite struct.
 *
 * @param encrypted_invite json formatted string containing encrypted key, iv and invite information
 * @param invite TrustDBle encrypted invite struct for holding the data from the file
 *
 * @return true when successfull otherwise false
 */
auto parse_encrypted_invite(const char *encrypted_invite, ENCRYPTED_STRUCT &invite) -> bool;

/**
 * @brief Reads a TrustDBle encrypted table invite from a json formatted string into a invite string.
 *
 * @param encrypted_invite json formatted string containing encrypted table invite information
 * @param invite Tstring for holding the data from the file
 *
 * @return true when successfull otherwise false
 */
auto parse_table_encrypted_invite(const char *encrypted_invite, std::string &invite) -> bool;

/**
 * @brief Reads a table invite from a json formatted string into a key_store struct.
 *
 * @param invite_key json formatted string containing table invite information
 * @param invite table invite struct for holding the data from the file
 *
 * @return true when successfull otherwise false
 */
auto parse_table_invite(const char *invite_key, KEY_STORE &invite) -> bool;

/**
 * @brief Reads a data contract from a json formatted string into a ENCRYPTED_STRUCT struct.
 *
 * @param encrypted_data_contract json formatted string containing data contract information
 * @param data_contract struct for holding the data from the encrypted data contract file
 *
 * @return true when successfull otherwise false
 */
auto parse_encrypted_data_contract(const char *encrypted_data_contract, ENCRYPTED_STRUCT &data_contract) -> bool;
/**
 * @brief Reads a data contract from a json formatted string into a data_contract struct.
 *
 * @param data_contract_key json formatted string containing data contract informationTstring
 * @param data_contract data contract struct for holding the data from the file
 *
 * @return true when successfull otherwise false
 */
auto parse_data_contract(const char *data_contract_key, REMOTE_DATA_CONTRACT &data_contract) -> bool;
/**
 * @brief Reads a table invite from a json formatted string into a federated_table struct.
 *
 * @param invite_key json formatted string containing table invite information
 * @param invite table invite struct for holding the data from the file
 *
 * @return true when successfull otherwise false
 */
auto parse_local_table_invite(const char *invite_key, FEDERATED_TABLE &invite) -> bool;

#endif // TRUSTDBLE_INVITE_H
