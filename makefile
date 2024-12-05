# Compiler and flags
CXX = g++
CXXFLAGS = -std=c++11

# Libraries
LIBS = -lcrypto -lssl

# Source files
SERVER_SRCS = server_side/server.cpp server_side/server_func.cpp Encrypt/Encryption.cpp
CLIENT_SRCS = client_side/client.cpp client_side/client_func.cpp Encrypt/Encryption.cpp

# Object files
SERVER_OBJS = $(SERVER_SRCS:.cpp=.o)
CLIENT_OBJS = $(CLIENT_SRCS:.cpp=.o)

# Executable names
SERVER_TARGET = server
CLIENT_TARGET = client

# Default target
all: $(SERVER_TARGET) $(CLIENT_TARGET)

# Link the server executable
$(SERVER_TARGET): $(SERVER_OBJS)
	$(CXX) $(CXXFLAGS) -o $(SERVER_TARGET) $(SERVER_OBJS) $(LIBS)

# Link the client executable
$(CLIENT_TARGET): $(CLIENT_OBJS)
	$(CXX) $(CXXFLAGS) -o $(CLIENT_TARGET) $(CLIENT_OBJS) $(LIBS)
# Compile source files into object files
%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Clean up build files
clean:
	rm -f $(SERVER_TARGET) $(CLIENT_TARGET) $(SERVER_OBJS) $(CLIENT_OBJS)

# Phony targets
.PHONY: all clean
