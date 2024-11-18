#include <iostream>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <cstring>

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
        if (recv(clientSocket, buffer, sizeof(buffer), 0) < 0) {
            std::cerr << "Receiving message failed!" << std::endl;
        } else {
            std::cout << "Message from client: " << buffer << std::endl;
        }

        // Send a response back to the client
        const char *response = "Message received";
        send(clientSocket, response, strlen(response), 0);

        // Close the client socket after communication
        close(clientSocket);
    }

    // Close server socket (never reached in this case since the server runs indefinitely)
    close(serverSocket);

    return 0;
}
