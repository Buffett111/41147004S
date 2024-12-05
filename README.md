# Simple Client-Server Application

This project demonstrates a simple client-server application using sockets for network communication. The server can handle multiple clients, allowing them to register, log in, and send text messages.

## Prerequisites

- Linux operating system
- g++ or any C++ compiler that supports POSIX sockets

## Files
- `server.cpp`: The server application
- `client.cpp`: The client application

## Compilation

### Using Command Line

1. Open a terminal.
2. Navigate to the directory containing `server.cpp` and `client.cpp`.
3. Compile the server:
    ```sh
    g++ server.cpp -o server
    ```
4. Compile the client:
    ```sh
    g++ client.cpp -o client
    ```

## Running the Application

1. Start the server:
    ```sh
    ./server
    ```
2. In a new terminal, start the client:
    ```sh
    ./client
    ```

The server will be running and waiting for client connections. The client can connect to the server and perform actions such as registering, logging in, and sending text messages.