# Compiler and flags
CXX = g++
CXXFLAGS = -std=c++17 -Wall -g  # -g for debugging symbols, -Wall for all warnings

# Directories
CLIENT_DIR = client
SERVER_DIR = server

# Source files for client and server
CLIENT_SRCS = $(CLIENT_DIR)/main.cpp $(CLIENT_DIR)/functions.cpp
SERVER_SRCS = $(SERVER_DIR)/main.cpp

# Object files (automatically generated from SRCS)
CLIENT_OBJS = $(CLIENT_SRCS:.cpp=.o)
SERVER_OBJS = $(SERVER_SRCS:.cpp=.o)

# Executables
CLIENT_EXEC = $(CLIENT_DIR)/email_client
SERVER_EXEC = $(SERVER_DIR)/email_server

# Default target
all: $(CLIENT_EXEC) $(SERVER_EXEC)

# Rule for building the client executable
$(CLIENT_EXEC): $(CLIENT_OBJS)
	$(CXX) $(CXXFLAGS) -o $(CLIENT_EXEC) $(CLIENT_OBJS)

# Rule for building the server executable
$(SERVER_EXEC): $(SERVER_OBJS)
	$(CXX) $(CXXFLAGS) -o $(SERVER_EXEC) $(SERVER_OBJS)

# Rule for compiling .cpp files into .o object files
$(CLIENT_DIR)/%.o: $(CLIENT_DIR)/%.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(SERVER_DIR)/%.o: $(SERVER_DIR)/%.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Clean up build files
clean:
	rm -f $(CLIENT_OBJS) $(SERVER_OBJS) $(CLIENT_EXEC) $(SERVER_EXEC)

# Rebuild everything
rebuild: clean all

# Ensure that we can also build without arguments
.PHONY: all clean rebuild
