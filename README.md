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
2. Navigate to the directory containing the project.
3. Just type ```sh make```

## Running the Application

1. Start the server(Default Port is 8080):
    ```sh
    ./server <PORT NUM>
    ```
2. In a new terminal, start the client:
    ```sh
    ./client <IP of the server> <PORT NUM>
    ```

The server will be running and waiting for client connections. The client can connect to the server and perform actions such as registering, logging in, and sending text messages.