#ifndef ENCRYPTION_H
#define ENCRYPTION_H
#include <openssl/evp.h>
#include <openssl/aes.h>
#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <string>
#include <map>
#include <cstdlib>
const unsigned char key[] = "5619045aeebb2222de4225d2f4ea9a0"; // 256-bit key
const unsigned char iv[] =  "c09bf68c8f9d175"; // 128-bit IV

std::string encrypt(const std::string& plaintext);
std::string decrypt(const std::string& ciphertext);
#endif // ENCRYPTION_H