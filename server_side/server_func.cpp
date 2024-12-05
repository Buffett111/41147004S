#include "server_func.h"
#include "Encryption.h"
#include <mutex>
#include <unistd.h>
#include <string.h>

std::unordered_map<int, std::string> client_usernames;
std::map<std::string, std::string> users;
std::mutex file_mutex;

size_t receive_file(int client_socket, const std::string& file_name) {
    std::lock_guard<std::mutex> lock(file_mutex); // Lock the mutex

    std::ofstream file("uploaded_" + file_name, std::ios::binary);
    if (!file.is_open()) {
        send_message(client_socket, "Error: Could not create file on server.\n");
        return -1;
    }
    size_t file_size=0;
    char buffer[1024];
    int bytes_read;
    while ((bytes_read = read(client_socket, buffer, sizeof(buffer))) > 0) {
        file.write(buffer, bytes_read);
        file_size+=bytes_read;
        if (bytes_read < sizeof(buffer)) {
            break; // End of file
        }
    }

    file.close();
    send_message(client_socket, "File uploaded successfully.\n");
    return file_size;
    
}

void send_file_to_receiver(int receiver_socket, const std::string& file_name) {
    std::ifstream file("uploaded_" + file_name, std::ios::binary);
    if (!file.is_open()) {
        send_message(receiver_socket, "Error: File not found on server.\n");
        return;
    }

    char buffer[1024];
    while (file.read(buffer, sizeof(buffer)) || file.gcount() > 0) {
        send(receiver_socket, buffer, file.gcount(), 0);
    }

    file.close();
    send_message(receiver_socket, "File transfer complete.\n");
}

void handle_file_transfer(int sender_socket, std::istringstream& iss) {
    std::string receiver_username, file_name;
    iss >> receiver_username >> file_name;

    // Save the file from the sender
    size_t file_size=receive_file(sender_socket, file_name);
    std::cout<<"Ask Client to accept file\n";
    // Notify the receiver
    for (auto& pair : client_usernames) {
        if (pair.second == receiver_username) {
            int receiver_socket = pair.first;

            // Ask the receiver if they want to accept the file
            std::lock_guard<std::mutex> lock(file_mutex); // Lock the mutex
            std::cout<<"Asking Client "<< client_usernames[receiver_socket] << " to accept file\n";
            send_message(receiver_socket, "FILEOFFER " + file_name + " from " + client_usernames[sender_socket] +" file_size "+ std::to_string(file_size) + "\n");

            // Wait for receiver's response
            char buffer[1024];
            int bytes_read = recv(receiver_socket, buffer, sizeof(buffer), 0);
            std::string response(buffer, bytes_read);

            if (response == "YES") {
                //send_message(receiver_socket, "Starting file transfer...");
                send_file_to_receiver(receiver_socket, file_name);
            } else {
                send_message(receiver_socket, "File transfer declined.");
                std::remove(("uploaded_" + file_name).c_str()); // Delete the file
            }

            return;
        }
    }

    // Receiver not found
    send_message(sender_socket, "Error: Receiver not online.\n");
    std::remove(("uploaded_" + file_name).c_str()); // Delete the file
}


void handle_video_upload(int sender_socket, std::istringstream& iss) {
    std::string receiver_username, file_name;
    iss >> receiver_username >> file_name;

    // Save the video from the sender
    size_t file_size = receive_file(sender_socket, file_name);
    if (file_size > 0) {
        std::cout << "Video file uploaded: " << file_name << " (" << file_size << " bytes)\n";
    } else {
        send_message(sender_socket, "Error uploading video file.");
        return;
    }

    // Notify the receiver to prepare for streaming
    for (auto& pair : client_usernames) {
        if (pair.second == receiver_username) {
            int receiver_socket = pair.first;

            std::string client_ip = ""; // Resolve the receiver's IP
            struct sockaddr_in addr;
            socklen_t addr_len = sizeof(addr);
            getpeername(receiver_socket, (struct sockaddr*)&addr, &addr_len);
            client_ip = inet_ntoa(addr.sin_addr);

            // Ask the receiver to start their video receiver
            send_message(receiver_socket, "Prepare video receiver\n");
            sleep(1); // Wait for the receiver to start the video receiver

            // Start streaming video to the receiver
            std::string streaming_command = "./stream_test/sender uploaded_" + file_name + " " + client_ip + " 5000";
            std::cout << "Streaming video: " << streaming_command << "\n";
            system(streaming_command.c_str());

            break;
        }
    }
}


// Function to send a message to the client
void send_message(int client_socket, const std::string& message) {
    std::string enc_message = encrypt(message);
    send(client_socket, enc_message.c_str(), enc_message.size(), 0);
}

void send_file(int client_socket, const std::string& file_name) {
    
}


// Function to load users from CSV file
void load_users() {
    std::ifstream file(USERS_FILE);
    if (!file.is_open()) {
        std::cerr << "Could not open users file." << std::endl;
        return;
    }

    std::string line;
    users.clear();
    while (std::getline(file, line)) {
        std::istringstream iss(line);
        std::string username, password;
        if (std::getline(iss, username, ',') && std::getline(iss, password, ',')) {
            users[username] = password;
        }
    }

    file.close();
}

// Function to save a new user to the CSV file
void save_user(const std::string& username, const std::string& password) {
    std::ofstream file(USERS_FILE, std::ios::app);
    if (!file.is_open()) {
        std::cerr << "Could not open users file." << std::endl;
        return;
    }

    file << username << "," << password << "\n";
    file.close();
}

// Function to save a chat record to the CSV file
void save_chat_record(const std::string& sender, const std::string& receiver, const std::string& message) {
    std::ofstream file(CHAT_RECORDS_FILE, std::ios::app);
    if (!file.is_open()) {
        std::cerr << "Could not open chat records file." << std::endl;
        return;
    }

    file << sender << "," << receiver << "," << message << "\n";
    file.close();
}

bool check_signup(const std::string& username, const std::string& password) {
    // Check if username already exists
    // If not, add the new user to the database
    if (users.find(username) != users.end()) {
        return false;
    }
    users[username] = password;
    save_user(username, password);
    return true;
}

bool check_login(const std::string& username, const std::string& password) {
    // Check if username exists and the password is correct
    if (users.find(username) == users.end()) {
        return false;
    }
    return users[username] == password;
}

// Function to handle client requests
void* handle_client(void* arg) {
    int client_socket = *(int*)arg;
    free(arg);

    char buffer[1024];
    bool is_running = true;
    std::string username, password;
    bool login = false;
    while (is_running) {
        memset(buffer, 0, sizeof(buffer));
        int bytes_read = read(client_socket, buffer, sizeof(buffer));

        if (bytes_read > 0) {
            std::string request(buffer);
            request = decrypt(request);
            std::cout << "Received: " << request << std::endl;
            std::istringstream iss(request);
            std::string command;
            iss >> command;

            if (command == "SIGNUP") {
                iss >> username >> password;
                if (check_signup(username, password)) {
                    send_message(client_socket, "Signup successful!\n");
                } else {
                    send_message(client_socket, "Username already exists!\n");
                }
            } else if (command == "LOGOUT") {

                std::cout << "Client logged out: " << client_socket << std::endl;
                send_message(client_socket, "Logout successful!\n");
                //client_usernames.erase(client_socket);
                login = false; // Ensure login status is reset
            } else if (command == "LOGIN") {
                iss >> username >> password;
                if (check_login(username, password)) {
                    client_usernames[client_socket] = username;
                    login = true;
                    send_message(client_socket, "Login successful!" + username + "\n");
                } else {
                    send_message(client_socket, "Invalid username or password.\n");
                }
            } else if (command == "MESSAGE") {
                if (!login) {
                    send_message(client_socket, "Please login first.\n");
                    continue;
                }
                std::string message;
                std::string receiver;
                iss >> receiver;
                // receive and ignore from username
                std::string from;
                iss >> from;
                std::getline(iss, message);
                // Remove leading receiver part of message
                username = client_usernames[client_socket];
                std::cout << username <<" To "<< receiver << ":\n" << message << " " <<"\n";
                save_chat_record(username, receiver, message);
                // Send to specific user
                for (auto& pair : client_usernames) {
                    if (pair.second == receiver) {
                        send_message(pair.first, username + ": " + message+"\n");
                    }
                }
            }else if (command == "UPLOADFILE") {
                handle_file_transfer(client_socket, iss);
            }else if (command == "UPLOADVIDEO") {
                handle_video_upload(client_socket, iss);
            }else if (command=="VIDEO"){ //"VIDEO " + receiver_username + " " + file_name;
                std::string receiver_username, file_name;
                iss >> receiver_username >> file_name;
                std::cout << "Sending video to " << receiver_username << "...\n";
                send_message(client_socket, "start_video "+file_name+"\n");//format start video video.mp4
                std::string command = "./stream_test/receiver"; 
                system(command.c_str());
            }else if (command == "EXIT") {
                is_running = false;
                //client_usernames.erase(client_socket);
                send_message(client_socket, "Goodbye!");
            } else {
                send_message(client_socket, "Unknown command.");
            }
        } else if (bytes_read == 0) {
            // Client disconnected
            std::cout << "Client disconnected: " << client_socket << std::endl;
            break;
        } else {
            perror("Error reading from socket");
            break;
        }
    }

    close(client_socket);
    return nullptr;
}