#ifndef CLIENT_FUNC_H
#define CLIENT_FUNC_H
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>
#include <netdb.h>
#include <atomic>
#include <ctime>
#include <sstream>
#include <vector>
#include <fstream>
#include <iostream>
#include <mutex>
#include <condition_variable>
#include "Encryption.h"
#pragma once
#define BUFFER_SIZE 1024


void upload_video_to_server(int client_socket, const std::string& receiver_username, const std::string& file_name);
void upload_file_to_server(int client_socket, const std::string& receiver_username, const std::string& file_name);
void* receive_messages(void* arg);
void receive_file(int client_socket, const std::string& file_name,const size_t file_size);
#endif // CLIENT_FUNC_H