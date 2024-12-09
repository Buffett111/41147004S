#include <iostream>
#include <vector>
#include "Encryption.h"
#include <fstream>
#include <string>
#include <vector>
using namespace std;
int main() {
    std::ifstream file("/home/buffett/Network-Application-Project/chat_records.txt", std::ios::binary);
    if (!file) {
        std::cerr << "Failed to open file." << std::endl;
        return 1;
    }

    const size_t bufferSize = 1024;
    char buffer[bufferSize];
    vector<string> v;
    while (file.read(buffer, bufferSize) || file.gcount() > 0) {
        std::string chunk(buffer, file.gcount());
        std::string encryptedChunk = encrypt(chunk);
        v.push_back(encryptedChunk);
        std::cout << "Encrypted chunk: " << encryptedChunk <<"\n\n\n";
    }
    for (int i = 0; i < v.size(); i++) {
        std::cout << "Encrypted chunk: len= "<<v[i].size() << " content:\n" << v[i] << "\n\n------------------------------\n\n";;
        std::string decryptedChunk = decrypt(v[i]);
        std::cout << "Decrypted chunk: len= "<<decryptedChunk.size() <<" content:\n"<< decryptedChunk <<  "\n\n------------------------------\n\n";;
    }

    file.close();
}
