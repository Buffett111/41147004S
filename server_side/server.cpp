#include "server_func.h"

int PORT = 8080;
extern std::unordered_map<int, std::string> client_usernames;
extern std::map<std::string, std::string> users;
int main(int argc, char* argv[]) {
    
    //let user assign port
    if(argc == 2) {
        PORT = atoi(argv[1]);
    }

    
    load_users(); // Load users from CSV file

    int server_fd, new_socket;
    struct sockaddr_in address;
    int addrlen = sizeof(address);

    // Create server socket
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("Socket failed");
        exit(EXIT_FAILURE);
    }

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    // Bind socket to the address
    if (bind(server_fd, (struct sockaddr*)&address, sizeof(address)) < 0) {
        perror("Bind failed");
        exit(EXIT_FAILURE);
    }

    // Start listening for connections
    if (listen(server_fd, 10) < 0) {
        perror("Listen failed");
        exit(EXIT_FAILURE);
    }

    std::cout << "Server is running on port " << PORT << std::endl;

    while (true) {
        if ((new_socket = accept(server_fd, (struct sockaddr*)&address, (socklen_t*)&addrlen)) < 0) {
            perror("Accept failed");
            exit(EXIT_FAILURE);
        }

        std::cout << "New client connected: " << new_socket << std::endl;

        // Create a new thread to handle the client
        pthread_t thread_id;
        int* socket_ptr = (int*)malloc(sizeof(int));
        *socket_ptr = new_socket;
        pthread_create(&thread_id, nullptr, handle_client, socket_ptr);
        pthread_detach(thread_id); // Detach thread to avoid blocking
    }

    return 0;
}
