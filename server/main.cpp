#include <iostream>
#include <cstring>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#define PORT 8080
#define BUFFER_SIZE 1024

int main() {
    int serverSocket, clientSocket;
    struct sockaddr_in serverAddr, clientAddr;
    socklen_t clientAddrSize = sizeof(clientAddr);
    char buffer[BUFFER_SIZE];

    // Create a socket
    serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSocket < 0) {
        std::cerr << "Socket creation failed!" << std::endl;
        return 1;
    }

    // Set up server address
    memset(&serverAddr, 0, sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = INADDR_ANY;
    serverAddr.sin_port = htons(PORT);

    // Bind the socket
    if (bind(serverSocket, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) < 0) {
        std::cerr << "Binding error: Port " << PORT << " is not available or already in use." << std::endl;
        return 1;
    }

    std::cout << "Server is running and binding to port " << PORT << "...\n";

    // Listen for incoming connections
    if (listen(serverSocket, 5) < 0) {
        std::cerr << "Listen failed!" << std::endl;
        return 1;
    }

    std::cout << "Waiting for connections...\n";

    // Accept incoming client connections in a loop
    while (true) {
        clientSocket = accept(serverSocket, (struct sockaddr *)&clientAddr, &clientAddrSize);
        if (clientSocket < 0) {
            std::cerr << "Accept failed!" << std::endl;
            continue;
        }

        std::cout << "Client connected!\n";

        // Receive message from client
        memset(buffer, 0, BUFFER_SIZE);
        int bytesReceived = recv(clientSocket, buffer, sizeof(buffer) - 1, 0);
        if (bytesReceived <= 0) {
            std::cerr << "Recv failed!" << std::endl;
            close(clientSocket);
            continue;
        }

        buffer[bytesReceived] = '\0';
        std::cout << "Received: " << buffer;

        // Check if login command
        std::string receivedMessage(buffer);
        if (receivedMessage.find("LOGIN") == 0) {
            std::cout << "Authentication requested...\n";
            std::string response = "OK\n";
            send(clientSocket, response.c_str(), response.size(), 0);
        } else {
            std::string errorResponse = "ERR\n";
            send(clientSocket, errorResponse.c_str(), errorResponse.size(), 0);
        }

        close(clientSocket);  // Close the connection
    }

    close(serverSocket);
    return 0;
}
