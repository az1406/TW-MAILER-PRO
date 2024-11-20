# Compiler and flags
CC = g++
CFLAGS = -g -Wall -Wextra -O -std=c++2a -pthread

# Directories
CLIENT_DIR = client
SERVER_DIR = server

# Source files for client and server
CLIENT_SRCS = $(CLIENT_DIR)/main.cpp $(CLIENT_DIR)/functions.cpp
SERVER_SRCS = $(SERVER_DIR)/main.cpp $(SERVER_DIR)/functions.cpp

# Object files (automatically generated from SRCS)
CLIENT_OBJS = $(CLIENT_SRCS:.cpp=.o)
SERVER_OBJS = $(SERVER_SRCS:.cpp=.o)

# Executables (located outside of the client and server directories)
CLIENT_EXEC = twmailer-client
SERVER_EXEC = twmailer-server

# Libraries (if needed, you can adjust this)
LIBS = -lldap -llber

# Default target
all: $(CLIENT_EXEC) $(SERVER_EXEC)

# Rule for building the client executable (outside the client folder)
$(CLIENT_EXEC): $(CLIENT_OBJS)
	$(CC) $(CFLAGS) -o $(CLIENT_EXEC) $(CLIENT_OBJS) $(LIBS)

# Rule for building the server executable (outside the server folder)
$(SERVER_EXEC): $(SERVER_OBJS)
	$(CC) $(CFLAGS) -o $(SERVER_EXEC) $(SERVER_OBJS) $(LIBS)

# Rule for compiling .cpp files into .o object files for the client
$(CLIENT_DIR)/%.o: $(CLIENT_DIR)/%.cpp
	$(CC) $(CFLAGS) -c $< -o $@

# Rule for compiling .cpp files into .o object files for the server
$(SERVER_DIR)/%.o: $(SERVER_DIR)/%.cpp
	$(CC) $(CFLAGS) -c $< -o $@

# Clean up build files
clean:
	rm -f $(CLIENT_OBJS) $(SERVER_OBJS) $(CLIENT_EXEC) $(SERVER_EXEC)

# Rebuild everything
rebuild: clean all

# Ensure that we can also build without arguments
.PHONY: all clean rebuild
