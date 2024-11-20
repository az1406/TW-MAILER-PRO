#include <iostream>
#include <cstring>
#include <cstdlib>
#include <unistd.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include "functions.h"

// Use the `std` namespace to avoid prefixing with std::
using namespace std;

int main(int argc, char **argv) {
    const size_t BUFFER_SIZE = 1024;
    int client_socket;
    char buffer[BUFFER_SIZE];
    struct sockaddr_in address;
    int size;
    bool isLogin = false;

    if ((client_socket = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        perror("Error creating socket");
        exit(EXIT_FAILURE);
    }

    memset(&address, 0, sizeof(address));
    address.sin_family = AF_INET;

    if (argc != 3) {
        cout << "Usage: ./twmailer-client <ip> <port>\n";
        exit(EXIT_FAILURE);
    }

    if (inet_pton(AF_INET, argv[1], &address.sin_addr) == 0) {
        cout << "Invalid IP address\n";
        exit(EXIT_FAILURE);
    }

    try {
        int port = stoi(argv[2]);
        if (port < 1024 || port > 65535) {
            cout << "Port must be between 1024 and 65535\n";
            exit(EXIT_FAILURE);
        }
        address.sin_port = htons(port);
    } catch (invalid_argument &) {
        cout << "Invalid port\n";
        exit(EXIT_FAILURE);
    }

    if (connect(client_socket, (struct sockaddr *)&address, sizeof(address)) == -1) {
        perror("Cannot connect to server");
        exit(EXIT_FAILURE);
    }

    cout << "Connected to server (" << inet_ntoa(address.sin_addr) << ")\n";

    size = recv(client_socket, buffer, BUFFER_SIZE - 1, 0);
    if (size > 0) {
        buffer[size] = '\0';
        cout << buffer;
    }

    while (true) {
        cout << ">> ";
        if (fgets(buffer, BUFFER_SIZE, stdin) != NULL) {
            int len = strlen(buffer);
            if (buffer[len - 1] == '\n') buffer[len - 1] = '\0'; // Remove trailing newline

            if (strcasecmp(buffer, "quit") == 0) {
                if (client_socket != -1) {
                    shutdown(client_socket, SHUT_RDWR);
                    close(client_socket);
                }
                exit(0);
            } else if (strcasecmp(buffer, "login") == 0) {
                string hs = login();
                send(client_socket, hs.c_str(), hs.size(), 0);
                size = recv(client_socket, buffer, BUFFER_SIZE - 1, 0);
                buffer[size] = '\0';
                if (printLogin(buffer)) isLogin = true;
            } else if (strcasecmp(buffer, "list") == 0) {
                if (isLogin) {
                    string hs = list();
                    send(client_socket, hs.c_str(), hs.size(), 0);
                    size = recv(client_socket, buffer, BUFFER_SIZE - 1, 0);
                    buffer[size] = '\0';
                    printList(buffer);
                } else {
                    cout << "Please login first\n";
                }
            } else if (strcasecmp(buffer, "send") == 0) {
                if (isLogin) {
                    string hs = send();
                    send(client_socket, hs.c_str(), hs.size(), 0);
                    size = recv(client_socket, buffer, BUFFER_SIZE - 1, 0);
                    buffer[size] = '\0';
                    printReply(buffer);
                } else {
                    cout << "Please login first\n";
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
                    cout << "Please login first\n";
                }
            } else {
                cout << "Unknown command\n";
            }
        }
    }

    return 0;
}
