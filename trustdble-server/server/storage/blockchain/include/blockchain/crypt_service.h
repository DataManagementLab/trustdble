#ifndef CRYPTO_SERVICE
#define CRYPTO_SERVICE

#include "adapter_factory/adapter_factory.h"
static const size_t HASH_SIZE = 32;
static const size_t KEY_SIZE = 32;
static const size_t IV_SIZE = 16;
static const size_t PUBLIC_KEY_SIZE = 736;
/**
   * @brief The method encodes the given plaintext with the provided key with a symmetric AES algortihm
   *        and returns the ciphertext
   *
   * @param[in] data the plaintext that has to be encrypted
   * @param[in] data_size the size plaintext that has to be encrypted
   * @param[in] key piece of information used in combination with aes algorithm
   *                to transform plaintext into ciphertext and vice versa.
   * @param[in] stringiv the initialization vector(IV)
   * @param[in] encrypted_data the encrypted plaintext (ciphertext)
   * @return the size encrypted plaintext (ciphertext)
   */
size_t encrypt(const unsigned char* data, int data_size,unsigned char * key,unsigned char * iv,unsigned char* encrypted_data);

/**
   * @brief The method decodes the given plaintext with the provided key with a symmetric AES algortihm
+   *        and decodes the given ciphertext
   *
   * @param[in] encrypted_data the ciphertext that has to be decoded
   * @param[in] encrypted_data_size the size of ciphertext that has to be decoded
   * @param[in] key a piece of information used in combination with aes algorithm
   *                to transform ciphertext into plaintext and vice versa.
   * @param[in] stringiv the initialization vector(IV)
   * @param[in] data the original plaintext
   * @return the size of original plaintext
   */
size_t decrypt(const unsigned char* encrypted_data, int encrypted_data_size,unsigned char * key,unsigned char * iv,unsigned char* data);
/**
   * @brief The method converts hex (Hexadecimal) string to string
   *
   * @param[in] in the hex (Hexadecimal) string that should be converted to string
   * @param[in] data the converted hex to string
   * @return the converted string
   */
auto hexToCharArray(const std::string &in, unsigned char *data) -> int ;

/**
   * @brief The method converts the string to hex (Hexadecimal) string
   *
   * @param[in] in the string that should be converted to hex (Hexadecimal) string
   * @param[in] length the length of string that should be converted to hex (Hexadecimal) string
   * @return the converted hex (Hexadecimal) string
   */
auto charArrayToHex(const unsigned char *data, unsigned length) -> std::string ;

/**
 * @brief Generates the SHA256 hash of the provided data.
 *
 * @param message The data whose hash will be generated
 * @param message_len The length of the data
 * @param hash A pointer to the generated hash
 * @param hash_len The length of the generated hash
 * @return status code (0 success, 1 failure)
 */
auto hash_sha256(const unsigned char *data, size_t data_len, unsigned char *hash, unsigned int *hash_len) -> int;

/**
 * @brief Encrypts provided data using provided public key.
 *
 * @param data data the plaintext that has to be encrypted
 * @param data_len The length of the data
 * @param public_key public key piece of information used in combination with rsa algorithm
   *                to transform plaintext into ciphertext and vice versa.
   *
 * @return encrypted data and it's  size using BYTES structs
 */

auto encrypt_public(const unsigned char* data , size_t data_len,const char* public_key)-> BYTES;
/**
 * @brief Decrypts the provided data using private key.
 *
 * @param[in] data The data which should be decrypted
 * @param[in] data_len The length of the data
 * @param[in] config_configuration_path configuration path to extract the address of private key
 * @param[in] server_number number of the server used as part of the passphrase of the private key
 *
 * @return decrypted data and it's size using BYTES structure
 */
auto decrypt_private(const unsigned char* data,size_t data_len, const char* config_configuration_path, const char* server_number,  unsigned char* decrypted_data)-> int;

/**
 * @brief validate provided public key.
 * @param[in] public_key public key piece of information used in combination with rsa algorithm
   *                to transform plaintext into ciphertext and vice versa.
 * @return status (true public key is valid,false public key is not valid)
 */
auto validate_publickey(const char* public_key) ->bool;

/**
 * @brief check wether the string is base64 encoded or not
 *
 * @param str The base64-encoded string to be checked.
 *
 * @return true is str is base64 encoded otherwise false
 */
auto is_base64_encoded(std::string str) -> bool;
#endif