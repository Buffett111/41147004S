#ifndef SERVER_FUNC_H
#define SERVER_FUNC_H

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <map>
#include <unordered_map>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <cstdlib>
#include <algorithm>
#include <vector>
#include <cstring>
#include <openssl/evp.h>
#include <openssl/aes.h>
#include <sys/types.h>
#include <sys/stat.h>

// Function declarations
void load_users();
void save_user(const std::string& username, const std::string& password);
void save_chat_record(const std::string& sender, const std::string& receiver, const std::string& message);
bool check_signup(const std::string& username, const std::string& password);
bool check_login(const std::string& username, const std::string& password);
void send_message(int client_socket, const std::string& message);
void* handle_client(void* arg);
void send_txt(int client_socket, const std::string& filename);
void handle_video_upload(int sender_socket, std::istringstream& iss);
void handle_file_transfer(int sender_socket, std::istringstream& iss);
void send_file_to_receiver(int receiver_socket, const std::string& file_name);
size_t receive_file(int client_socket, const std::string& file_name);

#define USERS_FILE "data/users.csv"
#define CHAT_RECORDS_FILE "data/chat_records.csv"
#endif // SERVER_FUNC_H
