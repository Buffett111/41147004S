# Advanced Client-Server Application

This project demonstrates an advanced client-server application using sockets for network communication. The server can handle multiple clients, allowing them to register, log in, and communicate with each other. Clients can send text messages, transfer files, and even stream video with other users. The application leverages OpenSSL for secure communication and OpenCV for video streaming capabilities.

## Prerequisites

Before you begin, ensure you have met the following requirements:

- **C++ Compiler**: Ensure you have a C++ compiler that supports C++11 or later. For example, `g++` (GNU Compiler Collection).

- **OpenSSL**: Install the OpenSSL library for cryptographic functions.
```sh
  sudo apt-get install libssl-devCV
```

- **OpenCV**: Install the OpenCV library for computer vision functions in video streaming.
```sh
sudo apt-get install libopencv-dev
```

- **Boost**: Install the Boost libraries
```sh
sudo apt-get install libboost-all-dev
```

- **Make**: Ensure you have `make` installed to build the project using the provided Makefile.
```sh
sudo apt-get install make
```

- **Git**: Ensure you have Git installed to clone the repository and manage version control
```sh
sudo apt-get install git
```

## Files
- `server.cpp`: The server application
- `client.cpp`: The client application

## Installation
1. Clone the repository:
```sh
git clone https://github.com/Buffett111/Network-Application-Project.git
cd Network-Application-Project
```
2. Build the project
```sh
make
```

## Usage

1. Start the server(Default Port is 8080):
    ```sh
    ./server <PORT NUM>
    ```
2. In a new terminal, start the client:
    ```sh
    ./client <IP of the server> <PORT NUM>
    ```

The server will be running and waiting for client connections. The client can connect to the server and perform actions such as registering, logging in, and sending text messages.

