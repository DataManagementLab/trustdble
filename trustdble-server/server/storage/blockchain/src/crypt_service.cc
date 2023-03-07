#include "blockchain/crypt_service.h"
#include "trustdble_utils/encoding_helpers.h"
#include <iomanip>
#include <iostream>
#include <openssl/conf.h>
#include <openssl/err.h>
#include <openssl/evp.h>
#include <openssl/rand.h>
#include <string.h>
#include <string>
#include <openssl/pem.h>
#include <openssl/rsa.h>
#include <fstream>

void handleErrors(void) {
  ERR_print_errors_fp(stderr);
  abort();
}

size_t encrypt(const unsigned char *data, int data_size, unsigned char *key,
               unsigned char *iv, unsigned char *encrypted_data) {
  EVP_CIPHER_CTX *ctx;
  int len;
  size_t encrypted_data_size;
  /* Create and initialise the context */
  if (!(ctx = EVP_CIPHER_CTX_new())){
    ERR_print_errors_fp(stderr);
    return 0;
  }

  /*
   * Initialise the encryption operation.
   */
  if (1 != EVP_EncryptInit_ex(ctx, EVP_aes_256_cbc(), NULL, key, iv)){
    ERR_print_errors_fp(stderr);
    return 0;
  }
  /*
   * Provide the message to be encrypted, and obtain the encrypted output.
   * EVP_EncryptUpdate can be called multiple times if necessary
   */
  if (1 != EVP_EncryptUpdate(ctx, encrypted_data, &len, data, data_size)){
    ERR_print_errors_fp(stderr);
    return 0;
  }
  encrypted_data_size = len;
  /*
   * Finalise the encryption. Further ciphertext bytes may be written at
   * this stage.
   */
  if (1 != EVP_EncryptFinal_ex(ctx, encrypted_data + len, &len)){
    ERR_print_errors_fp(stderr);
    return 0;
  }
  encrypted_data_size += len;

  /* Clean up */
  EVP_CIPHER_CTX_free(ctx);

  return encrypted_data_size;
}

size_t decrypt(const unsigned char *encrypted_data, int encrypted_data_size,
               unsigned char *key, unsigned char *iv, unsigned char *data) {
  EVP_CIPHER_CTX *ctx;
  int len;
  size_t data_size;
  /* Create and initialise the context */
  if (!(ctx = EVP_CIPHER_CTX_new())){
    ERR_print_errors_fp(stderr);
    return 0;
  }

  /*
   * Initialise the decryption operation.
   */
  if (1 != EVP_DecryptInit_ex(ctx, EVP_aes_256_cbc(), NULL, key, iv)){
    ERR_print_errors_fp(stderr);
    return 0;
  }

  /*
   * Provide the message to be decrypted, and obtain the plaintext output.
   * EVP_DecryptUpdate can be called multiple times if necessary.
   */
  if (1 !=EVP_DecryptUpdate(ctx, data, &len, encrypted_data, encrypted_data_size)){
    ERR_print_errors_fp(stderr);
    return 0;
  }
  data_size = len;

  /*
   * Finalise the decryption. Further plaintext bytes may be written at
   * this stage.
   */
  if (1 != EVP_DecryptFinal_ex(ctx, data + len, &len)){
    ERR_print_errors_fp(stderr);
    return 0;
  }
  data_size += len;

  /* Clean up */
  EVP_CIPHER_CTX_free(ctx);

  return data_size;
}

auto hash_sha256(const unsigned char *data, size_t data_len,
                 unsigned char *hash, unsigned int *hash_len) -> int {
  EVP_MD_CTX *mdctx;

  if ((mdctx = EVP_MD_CTX_new()) == NULL)
    handleErrors();

  if (1 != EVP_DigestInit_ex(mdctx, EVP_sha256(), NULL))
    handleErrors();

  if (1 != EVP_DigestUpdate(mdctx, data, data_len))
    handleErrors();

  if (1 != EVP_DigestFinal_ex(mdctx, hash, hash_len))
    handleErrors();

  EVP_MD_CTX_free(mdctx);

  return 0;
}
auto encrypt_public(const unsigned char* data , size_t data_len,const char* public_key)-> BYTES
{
    BIO *bio = BIO_new_mem_buf(public_key, -1);
    RSA* rsa = PEM_read_bio_RSA_PUBKEY(bio, NULL, NULL, NULL);
    int rsa_len = RSA_size(rsa);
    unsigned char* encrypted = (unsigned char*)malloc(rsa_len + 1);
    int encrypted_len = RSA_public_encrypt(data_len, data, encrypted, rsa, RSA_PKCS1_PADDING);
    BYTES encrypted_bytes= BYTES(encrypted,encrypted_len );
    RSA_free(rsa);
    CRYPTO_cleanup_all_ex_data();
    return encrypted_bytes;
}

auto decrypt_private(const unsigned char* data,size_t data_len, const char* config_configuration_path, const char* server_number, unsigned char* decrypted_data)-> int
{
    // extract the address of private key file from config_path
    std::string config_path_str(config_configuration_path);
    std::string pr_key_subpath = config_path_str.substr(0, config_path_str.find("/configs"));
    std::string pr_key_name="/keys/tdb-server-key.pem";
    std::string pr_key_file_name = pr_key_subpath+pr_key_name;

    //generate privte key file's password
    std::string keypass= std::string(server_number)+"tdb-server-key";
    char * keypass_byte= const_cast<char*>( keypass.c_str() );

    //open the private key file
    FILE *fp = fopen(pr_key_file_name.c_str(), "r");
    RSA *rsa = PEM_read_RSAPrivateKey(fp, NULL, NULL, keypass_byte);
    if(rsa==NULL)
       ERR_print_errors_fp(stderr);
    fclose(fp);
    int rsa_outlen  = RSA_private_decrypt(data_len, data, decrypted_data,rsa, RSA_PKCS1_PADDING);
    RSA_free(rsa);
    return rsa_outlen;
}
auto validate_publickey(const char* public_key) ->bool {
  BIO *bio = BIO_new_mem_buf(public_key, -1);
  RSA* rsa = PEM_read_bio_RSA_PUBKEY(bio, NULL, NULL, NULL);
  if(rsa==nullptr){
    RSA_free(rsa);
    CRYPTO_cleanup_all_ex_data();
    return false;
  }
  const BIGNUM *e=NULL;
  const BIGNUM *n=NULL;
  const BIGNUM *d=NULL;
  RSA_get0_key(rsa, &n,&e,&d);

  bool rc = (rsa && !d && n && e) && (BN_is_odd(e) && !BN_is_one(e));

  RSA_free(rsa);
  CRYPTO_cleanup_all_ex_data();
  return rc;
}
auto is_base64_encoded(std::string str) -> bool
{
  const std::regex base_regex("^(?:[A-Za-z0-9+/]{4})*(?:[A-Za-z0-9+/]{2}==|[A-Za-z0-9+/]{3}=)?$");
  std::smatch base_match;
  if ((bool) std::regex_match(str,base_match,base_regex)==false){
    return false;
  }
  else if (str.length()%4!=0)
    return false;
  else
    return true;
}
