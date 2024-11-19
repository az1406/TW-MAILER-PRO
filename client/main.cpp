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
    bool isEntryCorrect;
    bool isLogin = false;
    bool isSend = false;
    bool isList = false;
    bool isMessage = false;
    bool isDel = false;
    string hs;

    if ((client_socket = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        perror("Fehler beim erstellen des Sockets");
        exit(EXIT_FAILURE);
    }

    memset(&address, 0, sizeof(address));
    address.sin_family = AF_INET;

    // ARGUMENT HANDLING
    if (argc != 3) {
        cout << "Usage: ./twmailer-client <ip> <port>\n";
        exit(EXIT_FAILURE);
    } else {
        //checks if given ip-address is viable
        if (inet_pton(AF_INET, argv[1], &address.sin_addr) == 0) {
            cout << "Input IP-address is not valid\n";
            exit(EXIT_FAILURE);
        }

        try {
            //checks if port is in suitable range
            if (stoi(argv[2]) < 1024 || stoi(argv[2]) > 65535) {
                cout << "Input Port is not in usable port range\n";
                exit(EXIT_FAILURE);
            }
        } catch (invalid_argument&) {
            cout << "Port was not a number\n";
            exit(EXIT_FAILURE);
        }

        address.sin_port = htons(stoi(argv[2]));
    }

    if (connect(client_socket, (struct sockaddr *)&address, sizeof(address)) == -1) {
        perror("Server konnte nicht erreicht werden");
        exit(EXIT_FAILURE);
    }

    printf("Connection with server (%s) established\n", inet_ntoa(address.sin_addr));

    size = recv(client_socket, buffer, BUFFER_SIZE - 1, 0);
    switch (size) {
        case -1:
            perror("recv error");
            break;
        case 0:
            printf("Server closed remote socket\n");
            break;
        default:
            buffer[size] = '\0';
            printf("%s", buffer);
    }

    do {
        printf(">> ");
        if (fgets(buffer, BUFFER_SIZE, stdin) != NULL) {
            int size = strlen(buffer);
            if (buffer[size - 2] == '\r' && buffer[size - 1] == '\n') {
                size -= 2;
                buffer[size] = 0;
            } else if (buffer[size - 1] == '\n') {
                --size;
                buffer[size] = 0;
            }

            if (strcasecmp(buffer, "quit") == 0) {
                if (client_socket != -1) {
                    if (shutdown(client_socket, SHUT_RDWR) == -1) {
                        perror("shutdown create_socket");
                    }
                    if (close(client_socket) == -1) {
                        perror("close create_socket");
                    }
                }
                exit(0);
            } else if (strcasecmp(buffer, "login") == 0) {
                hs = login();
                send(client_socket, hs.c_str(), hs.size(), 0);
                size = recv(client_socket, buffer, BUFFER_SIZE - 1, 0);
                buffer[size] = '\0';
                isEntryCorrect = printLogin(buffer);
                if (isEntryCorrect) {
                    isLogin = true;
                    cout << "Connected\n";
                }
            } else if (strcasecmp(buffer, "send") == 0) {
                if (isLogin) {
                    hs = send();
                    send(client_socket, hs.c_str(), hs.size(), 0);
                    size = recv(client_socket, buffer, BUFFER_SIZE - 1, 0);
                    buffer[size] = '\0';
                    printReply(buffer);
                } else {
                    cout << "Please login first\n";
                }
            }
        }
    } while (true);

    return 0;
}
