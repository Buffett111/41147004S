#include "client_func.h"

extern std::mutex file_transfer_mutex;
extern std::condition_variable file_transfer_cv;
bool file_transfer_in_progress = false;

std::atomic<bool> keep_running(true);
std::mutex file_mutex;
std::string username, password;
std::string users;
bool login = false;
bool ask_permit=false;

void upload_video_to_server(int client_socket, const std::string& receiver_username, const std::string& file_name) {
    std::ifstream file(file_name, std::ios::binary);
    if (!file.is_open()) {
        std::cout << "Error: Could not open video file." << std::endl;
        return;
    }

    // Send the UPLOADVIDEO command
    std::string command = "UPLOADVIDEO " + receiver_username + " " + file_name;
    command = encrypt(command);
    send(client_socket, command.c_str(), command.size(), 0);

    // Send the video content
    char buffer[BUFFER_SIZE];
    while (file.read(buffer, sizeof(buffer)) || file.gcount() > 0) {
        send(client_socket, buffer, file.gcount(), 0);
    }
    file.close();

    std::cout << "Video uploaded successfully to the server.\n";
}


void upload_file_to_server(int client_socket, const std::string& receiver_username, const std::string& file_name) {
    std::ifstream file(file_name, std::ios::binary);
    if (!file.is_open()) {
        std::cout << "Error: Could not open file." << std::endl;
        return;
    }

    // Send the UPLOADFILE command
    std::string command = "UPLOADFILE " + receiver_username + " " + file_name;
    command = encrypt(command);
    send(client_socket, command.c_str(), command.size(), 0);

    // Send the file content
    char buffer[BUFFER_SIZE];
    while (file.read(buffer, sizeof(buffer)) || file.gcount() > 0) {
        send(client_socket, buffer, file.gcount(), 0);
    }
    file.close();

    std::cout << "File uploaded successfully to the server.\n";
}
void* receive_messages(void* arg) {
    int client_socket = *(int*)arg;
    char buffer[BUFFER_SIZE];
    std::string partial_message;

    while (keep_running) {
        memset(buffer, 0, sizeof(buffer));
        int bytes_read = recv(client_socket, buffer, sizeof(buffer), 0);

        if (bytes_read > 0) {
            partial_message += decrypt(std::string(buffer, bytes_read));

            // Process complete messages
            size_t pos;
            while ((pos = partial_message.find('\n')) != std::string::npos) {
                std::string message = partial_message.substr(0, pos);
                partial_message.erase(0, pos + 1);

                // Handle specific server messages
                if (message.find("FILEOFFER") != std::string::npos) {
                    // Extract file name and sender details
                    //format FILEOFFER chat_records.txt from Andy file_size 1000
                    std::string file_name = message.substr(message.find(" ") + 1, message.find(" from") - message.find(" ") - 1);
                    std::string file_size_string=message.substr(message.find("file_size")+10,message.find("\n")-message.find("file_size")-1);
                    std::cout << "file_size_string is: " << file_size_string << "\n";
                    size_t file_size = std::stoi(file_size_string);
                    //size_t file_size = std::stoi(message.substr(message.find(" ", message.find(" from") + 1) + 1));
                    std::cout << "\nfile_name is: " << file_name << "\n";
                    std::cout << "file_size is: " << file_size << "\n";
                    ask_permit=true;
                    std::cout << "\n[Server]: " << message << "\nAccept the file? (YES/NO): ";
                    
                    std::string response="YES";
                    // std::getline(std::cin, response);
                    // std::cout << "original response: "<<response<<"\n";
                    // //turn response to uppercase
                    // for(int i = 0; i < response.size(); i++) 
                    //     response[i] = toupper(response[i]);
                    // // Send response to server
                    // if(response=="Y")
                    //     response = "YES";
                    
                    std::cout << "Sending response: " << response << std::endl;
                    send(client_socket, response.c_str(), response.size(), 0);

                    if (response == "YES") {
                        // Set file transfer flag
                        {
                            std::unique_lock<std::mutex> lock(file_transfer_mutex);
                            file_transfer_in_progress = true;
                        }

                        // Start receiving the file
                        receive_file(client_socket, file_name,file_size);

                        // Reset file transfer flag
                        {
                            std::unique_lock<std::mutex> lock(file_transfer_mutex);
                            file_transfer_in_progress = false;
                            file_transfer_cv.notify_all(); // Notify waiting threads
                        }
                    } else {
                        std::cout << "File transfer declined." << std::endl;
                    }
                } else if (message.find("Login successful!") != std::string::npos) {
                    login = true;
                    users = message.substr(message.find("!") + 1);
                    std::cout << "You are logged in as: " << users << std::endl;
                } else if (message.find("Logout successful!") != std::string::npos) {
                    login = false;
                    std::cout << "You have logged out." << std::endl;
                } else if (message.find("Prepare video receiver") != std::string::npos) { //format start video video.mp4
                    std::cout << "Preparing to receive video stream..." << std::endl;
                    std::string receiver_command = "./stream_test/receiver 5000";
                    system(receiver_command.c_str());
                } else if (message.find("end video") != std::string::npos) {
                    std::cout << "Video streaming ended." << std::endl;
                } else {
                    std::cout << "\n[Server]: " << message << std::endl;
                }

                std::cout << "> ";
                std::cout.flush();
            }
        } else if (bytes_read == 0) {
            std::cout << "\nServer disconnected." << std::endl;
            keep_running = false;
            break;
        } else {
            perror("Error reading from socket");
            keep_running = false;
            break;
        }
    }

    return nullptr;
}

void receive_file(int client_socket, const std::string& file_name,const size_t file_size) {
    //create file called downloaded_filename
    std::lock_guard<std::mutex> lock(file_mutex); // Lock the mutex
    std::string local_file_name = "downloaded_" + file_name;
    //std::cout <<"Opening file: "<<local_file_name<<"\n";
    std::ofstream file(local_file_name, std::ios::binary);

    //std::cout << "Trying to Download file: " << file_name << std::endl;
    if (!file.is_open()) {
        std::cerr << "Error: Could not create file on client.\n";
        perror("Error details");
        return;
    }

    char buffer[1024];
    int bytes_read;
    size_t total_bytes = 0;

    std::cout << "Downloading file: " << file_name << std::endl;

    while (true) {
        {
            std::unique_lock<std::mutex> lock(file_transfer_mutex);
            if (!file_transfer_in_progress) {
                std::cout << "Error: File transfer flag not set. Aborting download.\n";
                break;
            }
        }

        bytes_read = recv(client_socket, buffer, sizeof(buffer), 0);
        // std::cout << "Bytes read: " << bytes_read << std::endl;
        // std::cout << "Total bytes: " << total_bytes << std::endl;
        // std::cout << "File size: " << file_size << std::endl;
        // std::cout << "Buffer: " << buffer << std::endl; // Print buffer contents

        if (bytes_read > 0) {
            file.write(buffer, bytes_read);
            total_bytes += bytes_read;

            // Display progress
            float progress = (float)total_bytes / file_size * 100.0f;
            std::cout << "\rProgress: " << (int)progress << "%";
            std::cout.flush();

            if ( (total_bytes >= file_size)  ) {
                break; // End of file
            }
        } else {
            std::cout << "\nFile transfer interrupted or completed.\n";
            break;
        }
    }

    file.close();
    std::cout << "\nDownload complete. File saved as 'downloaded_" << file_name << "'." << std::endl;
}
