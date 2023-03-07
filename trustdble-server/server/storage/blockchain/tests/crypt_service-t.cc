#include "blockchain/crypt_service.h"
#include "my_config.h"
#include <gtest/gtest.h>
#include <openssl/rand.h>


  TEST(EncryptDecrypt,EncryptDecryptString) {
    unsigned char * key= new unsigned char[KEY_SIZE];
    unsigned char * iv= new unsigned char[IV_SIZE];
    RAND_bytes(key, KEY_SIZE);
    RAND_bytes(iv, IV_SIZE);

    char input[] = "The quick brown fox jumps over the lazy dog";
    size_t input_len = strlen(input);
    unsigned char* plaintext= (unsigned char *)input;
    unsigned char* decrypted= new unsigned char[200];
    unsigned char* encrypted= new unsigned char[200];
    size_t len1 = encrypt(plaintext, input_len, key, iv, encrypted);
    size_t len2 = decrypt(encrypted, len1, key, iv, decrypted);

    std::string output = std::string(reinterpret_cast<char*>(decrypted), len2);

    EXPECT_EQ(std::string(input),output);
}
