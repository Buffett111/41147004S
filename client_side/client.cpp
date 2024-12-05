#include <iostream>
#include <pthread.h>
#include <fstream>

#include "client_func.h"

unsigned int PORT = 8080;
extern std::atomic<bool> keep_running;
extern std::string username, password;
extern std::string users;

std::mutex file_transfer_mutex;
std::condition_variable file_transfer_cv;
bool is_file_transfer_in_progress = false;

extern bool login;
extern bool ask_permit;

int main(int argc, char* argv[]) {
    int client_socket;
    struct sockaddr_in server_address;

    if (argc < 2) {
        std::cerr << "Usage: " << argv[0] << " <IP Address or URL> <PORT>" << std::endl;
        return -1;
    }
    //check if assign port
    if(argc == 3) {
        PORT = atoi(argv[2]);
    }

    // Resolve IP address from URL if necessary
    std::string server_ip = argv[1];
    struct hostent* host = gethostbyname(server_ip.c_str());
    if (host) {
        server_ip = inet_ntoa(*((struct in_addr*)host->h_addr_list[0]));
    }

    // Create socket
    if ((client_socket = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("Socket creation failed");
        return -1;
    }

    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(PORT);

    if (inet_pton(AF_INET, server_ip.c_str(), &server_address.sin_addr) <= 0) {
        perror("Invalid address");
        return -1;
    }

    // Connect to the server
    if (connect(client_socket, (struct sockaddr*)&server_address, sizeof(server_address)) < 0) {
        perror("Connection failed");
        return -1;
    }

    std::cout << "Connected to server at " << server_ip << std::endl;

    // Create a pthread to listen for incoming messages
    pthread_t recv_thread;
    if (pthread_create(&recv_thread, nullptr, receive_messages, &client_socket) != 0) {
        perror("Failed to create receiving thread");
        return -1;
    }

    while (keep_running) {
        system("clear");
        if (login) {
            std::cout << "\nWelcome " << users << "!";
            std::cout << "\n--- Menu ---\n";
            std::cout << "1. Signup (SIGNUP username password)\n";
            std::cout << "3. Send Message (MESSAGE <text>)\n";
            std::cout << "4. Logout (LOGOUT username)\n";
            std::cout << "5. Exit\n";
            std::cout << "6: send a file\n";
            std::cout << "Enter your choice: ";
        } else {
            std::cout << "\n--- Menu ---\n";
            std::cout << "1. Signup (SIGNUP username password)\n";
            std::cout << "2. Login (LOGIN username password)\n";
            std::cout << "5. Exit\n";
            std::cout << "Enter your choice: ";
        }

        std::string input;
        std::string command;
        std::getline(std::cin, input);
        if (input.empty()) {
            continue;
        } else if (input == "4") {
            if (!login) {
                std::cout << "You are not logged in." << std::endl;
                continue;
            }
            command = "LOGOUT " + username;
        } else if (input == "1" || input == "2") {
            if (input == "2" && login) {
                std::cout << "You are already logged in." << std::endl;
                continue;
            }
            std::cout << "Enter username: ";
            std::getline(std::cin, username);
            std::cout << "Enter password: ";
            std::getline(std::cin, password);
            if (input == "1") {
                command = "SIGNUP";
            } else if (input == "2" && !login) {
                command = "LOGIN";
            } else if (input == "4" && login) {
                command = "LOGOUT";
            }
            command = command + " " + username + " " + password;
        } else if (input == "3" && login) {
            std::string r_user;
            std::cout << "Enter receiver username: ";
            std::getline(std::cin, r_user);
            std::cout << "Enter message: ";
            std::getline(std::cin, command);
            command = "MESSAGE " + r_user + " " + username + " " + command;
        } else if (input == "5") {
            std::string command = "EXIT";
            command = encrypt(command);
            send(client_socket, command.c_str(), command.size(), 0);
            keep_running = false;
            break;
        } else if (input == "6" && login) {
            std::string receiver_username, file_name;
            std::cout << "Enter the receiver's username: ";
            std::getline(std::cin, receiver_username);
            std::cout << "Enter the path of the file to send: ";
            std::getline(std::cin, file_name);

            upload_file_to_server(client_socket, receiver_username, file_name);
            continue;
        }else if (input == "7" && login) {
            std::string receiver_username, file_name;
            std::cout << "Enter the receiver's username: ";
            std::getline(std::cin, receiver_username);
            std::cout << "Enter the path of the video to send: ";
            std::getline(std::cin, file_name);

            upload_video_to_server(client_socket, receiver_username, file_name);
            continue;
        }else if(input=="y"||input=="Y"){
            if(ask_permit){
                std::cout << "Sending responsefrom main: " << input << std::endl;
                send(client_socket, "YES", 3, 0);
            }
        
        } else {
            std::cout << "Invalid choice." << std::endl;
            continue;
        }

        std::cout << "Sending command: " << command << std::endl;
        std::string enc_command = encrypt(command);
        std::cout << "Encrypted C string: " << enc_command.c_str() << std::endl;
        send(client_socket, enc_command.c_str(), enc_command.size(), 0);
        std::cout << "Press Enter to continue...";
        std::cin.get(); // Wait for user to press Enter
    }

    // Wait for the receiving thread to finish
    pthread_join(recv_thread, nullptr);

    close(client_socket);
    return 0;
}
