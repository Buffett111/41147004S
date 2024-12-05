#include <iostream>
#include <string>
#include"Encryption.h"
int main() {
    std::string input;
    std::cout << "Enter a string to encrypt: ";
    std::getline(std::cin, input);

    std::string encrypted = encrypt(input);
    std::cout << "Encrypted string: " << encrypted << std::endl;

    std::string decrypted = decrypt(encrypted);
    std::cout << "Decrypted string: " << decrypted << std::endl;

    return 0;
}