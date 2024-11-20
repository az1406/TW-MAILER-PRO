#include <iostream>
#include <cstring>
#include <cstdlib>
#include <unistd.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include "functions.h"

using namespace std;

int main(int argc, char **argv) {
    const size_t BUFFER_SIZE = 1024;
    int client_socket;
    char buffer[BUFFER_SIZE];
    struct sockaddr_in address;
    bool isLogin = false;
// Create a TCP socket
    if ((client_socket = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        perror("Error creating socket");
        exit(EXIT_FAILURE);
    }

    memset(&address, 0, sizeof(address));
    address.sin_family = AF_INET;
// Check if enough arguments are passed (IP and port)
    if (argc != 3) {
        cerr << "Usage: ./twmailer-client <ip> <port>\n";
        exit(EXIT_FAILURE);
    }

    if (inet_pton(AF_INET, argv[1], &address.sin_addr) <= 0) {
        cerr << "Invalid IP address\n";
        exit(EXIT_FAILURE);
    }

    try {
        int port = stoi(argv[2]);
        if (port < 1024 || port > 65535) { // Ensure the port is within valid ran
            cerr << "Port must be between 1024 and 65535\n";
            exit(EXIT_FAILURE);
        }
        address.sin_port = htons(port); // Set the port in network byte order
    } catch (const invalid_argument &) {
        cerr << "Invalid port\n";
        exit(EXIT_FAILURE);
    }
// Attempt to connect to the server
    if (connect(client_socket, (struct sockaddr *)&address, sizeof(address)) == -1) {
        perror("Cannot connect to server");
        exit(EXIT_FAILURE);
    }

    cout << "Connected to server (" << inet_ntoa(address.sin_addr) << ")\n";

    int size = recv(client_socket, buffer, BUFFER_SIZE - 1, 0);
    if (size > 0) {
        buffer[size] = '\0';
        cout << buffer;
    }
// Main loop to handle user input and communication with server
    while (true) {
        cout << ">> ";
        if (fgets(buffer, BUFFER_SIZE, stdin) != nullptr) {
            int len = strlen(buffer);
            if (buffer[len - 1] == '\n') buffer[len - 1] = '\0'; // Remove trailing newline

            if (strcasecmp(buffer, "quit") == 0) {
                cout << "Exiting...\n";
                shutdown(client_socket, SHUT_RDWR);
                close(client_socket);
                break;
            } else if (strcasecmp(buffer, "login") == 0) { // Handle "login" command
                string hs = login(); 
                send(client_socket, hs.c_str(), hs.size(), 0);// Send login request
                size = recv(client_socket, buffer, BUFFER_SIZE - 1, 0);// Receive login response
                buffer[size] = '\0';
                isLogin = printLogin(buffer);
            } else if (strcasecmp(buffer, "list") == 0) {
    if (isLogin) {
        string hs = list();
        send(client_socket, hs.c_str(), hs.size(), 0);
        size = recv(client_socket, buffer, BUFFER_SIZE - 1, 0);
        buffer[size] = '\0';
        printList(buffer);  // Process and print the list of subjects
    } else {
        cerr << "Please login first\n";
    }
} else if (strcasecmp(buffer, "send") == 0) {
                if (isLogin) {
                    string hs = sendMail();
                    send(client_socket, hs.c_str(), hs.size(), 0);
                    size = recv(client_socket, buffer, BUFFER_SIZE - 1, 0);
                    buffer[size] = '\0';
                    printReply(buffer);
                } else {
                    cerr << "Please login first\n";
                }
            } else if (strcasecmp(buffer, "read") == 0) {
    if (isLogin) {
        string hs = read();
        send(client_socket, hs.c_str(), hs.size(), 0);
        size = recv(client_socket, buffer, BUFFER_SIZE - 1, 0);
        buffer[size] = '\0';
        printMessage(buffer); 
    } else {
        cout << "Please login first\n";
    }
} else if (strcasecmp(buffer, "del") == 0) {
                if (isLogin) {
                    string hs = del();
                    send(client_socket, hs.c_str(), hs.size(), 0);
                    size = recv(client_socket, buffer, BUFFER_SIZE - 1, 0);
                    buffer[size] = '\0';
                    printReply(buffer);
                } else {
                    cerr << "Please login first\n";
                }
            } else {
                cerr << "Unknown command\n";
            }
        }
    }
    return 0;
}
