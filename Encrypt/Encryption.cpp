#include "Encryption.h"
#include <openssl/evp.h>
#include <openssl/bio.h>
#include <openssl/buffer.h>
#include <cstring>

std::string base64_encode(const unsigned char* buffer, size_t length) {
    BIO* bio, *b64;
    BUF_MEM* bufferPtr;

    b64 = BIO_new(BIO_f_base64());
    bio = BIO_new(BIO_s_mem());
    bio = BIO_push(b64, bio);

    BIO_set_flags(bio, BIO_FLAGS_BASE64_NO_NL); // Don't add newline
    BIO_write(bio, buffer, length);
    BIO_flush(bio);
    BIO_get_mem_ptr(bio, &bufferPtr);

    std::string result(bufferPtr->data, bufferPtr->length);
    BIO_free_all(bio);
    return result;
}

std::string encrypt(const std::string& plaintext) {
    unsigned char ciphertext[4096];
    int out_len, ciphertext_len;

    EVP_CIPHER_CTX* ctx = EVP_CIPHER_CTX_new();
    EVP_EncryptInit_ex(ctx, EVP_aes_256_cbc(), nullptr, key, iv);

    EVP_EncryptUpdate(ctx, ciphertext, &out_len, (unsigned char*)plaintext.c_str(), plaintext.size());
    ciphertext_len = out_len;

    EVP_EncryptFinal_ex(ctx, ciphertext + out_len, &out_len);
    ciphertext_len += out_len;

    EVP_CIPHER_CTX_free(ctx);
    return base64_encode(ciphertext, ciphertext_len);
}


std::string base64_decode(const std::string& input) {
    BIO* bio, *b64;
    char* buffer = (char*)malloc(input.size());
    memset(buffer, 0, input.size());

    bio = BIO_new_mem_buf(input.c_str(), input.size());
    b64 = BIO_new(BIO_f_base64());
    bio = BIO_push(b64, bio);

    BIO_set_flags(bio, BIO_FLAGS_BASE64_NO_NL); // Don't expect newline
    int length = BIO_read(bio, buffer, input.size());
    BIO_free_all(bio);

    std::string result(buffer, length);
    free(buffer);
    return result;
}

std::string decrypt(const std::string& ciphertext) {
    unsigned char plaintext[4096];
    int out_len, plaintext_len;

    std::string decoded_ciphertext = base64_decode(ciphertext);

    EVP_CIPHER_CTX* ctx = EVP_CIPHER_CTX_new();
    EVP_DecryptInit_ex(ctx, EVP_aes_256_cbc(), nullptr, key, iv);

    EVP_DecryptUpdate(ctx, plaintext, &out_len, (unsigned char*)decoded_ciphertext.c_str(), decoded_ciphertext.size());
    plaintext_len = out_len;

    EVP_DecryptFinal_ex(ctx, plaintext + out_len, &out_len);
    plaintext_len += out_len;

    EVP_CIPHER_CTX_free(ctx);
    return std::string((char*)plaintext, plaintext_len);
}
