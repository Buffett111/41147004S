#include "Encryption.h"
std::string encrypt(const std::string& plaintext) {
    // load_env(".env");
    // const unsigned char* key = (unsigned char*)std::getenv("KEY");
    // const unsigned char* iv = (unsigned char*)std::getenv("IV");

    //return plaintext;
    unsigned char ciphertext[4096];
    int out_len, ciphertext_len;

    EVP_CIPHER_CTX* ctx = EVP_CIPHER_CTX_new();
    EVP_EncryptInit_ex(ctx, EVP_aes_256_cbc(), nullptr, key, iv);

    EVP_EncryptUpdate(ctx, ciphertext, &out_len, (unsigned char*)plaintext.c_str(), plaintext.size());
    ciphertext_len = out_len;

    EVP_EncryptFinal_ex(ctx, ciphertext + out_len, &out_len);
    ciphertext_len += out_len;

    EVP_CIPHER_CTX_free(ctx);
    return std::string((char*)ciphertext, ciphertext_len);
}

std::string decrypt(const std::string& ciphertext) {
    // load_env(".env");
    // const unsigned char* key = (unsigned char*)std::getenv("KEY");
    // const unsigned char* iv = (unsigned char*)std::getenv("IV");
    
    // return ciphertext;
    unsigned char plaintext[4096];
    int out_len, plaintext_len;

    EVP_CIPHER_CTX* ctx = EVP_CIPHER_CTX_new();
    EVP_DecryptInit_ex(ctx, EVP_aes_256_cbc(), nullptr, key, iv);

    EVP_DecryptUpdate(ctx, plaintext, &out_len, (unsigned char*)ciphertext.c_str(), ciphertext.size());
    plaintext_len = out_len;

    EVP_DecryptFinal_ex(ctx, plaintext + out_len, &out_len);
    plaintext_len += out_len;

    EVP_CIPHER_CTX_free(ctx);
    return std::string((char*)plaintext, plaintext_len);
}