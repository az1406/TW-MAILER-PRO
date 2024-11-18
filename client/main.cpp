#include <iostream>
#include <cstring>
#include <unistd.h>
#include <ldap.h> 
#include <arpa/inet.h>
#include "functions.h"

#define PORT 8080

int main(int argc, char **argv) {
    std::cout << "\nInitializing email client...\n\n";
    int socket_fd;
    struct sockaddr_in address{};
    bool isAuthenticated = false, isQuit = false;

    // Create socket
    if ((socket_fd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        perror("Socket error");
        return 1;
    }

    try {
        auto address_info = get_address_info_from_command_args(argc, argv);
        address.sin_family = AF_INET;
        address.sin_port = htons(address_info.second);
        if (inet_aton(address_info.first.c_str(), &address.sin_addr) == 0) {
            std::cerr << "Invalid IP address format.\n";
            return 1;
        }

        // Connect to the server
        if (connect(socket_fd, (struct sockaddr *)&address, sizeof(address)) == -1) {
            perror("Connection error");
            return 1;
        }

        std::cout << "Connection established with " << inet_ntoa(address.sin_addr) << "\n";

        // Authentication loop
        while (!isAuthenticated && !isQuit) {
            std::string response = handleLoginCommand(socket_fd);
            if (response == "OK\n") {
                isAuthenticated = true;
                std::cout << "Authentication successful.\n";
            } else {
                std::cout << "Server response: " << response << "\n";
                isQuit = getUserInput("Retry? (yes/no): ") != "yes";
            }
        }

        // Exit after login test
        if (!isAuthenticated) {
            std::cout << "Exiting...\n";
        }
    } catch (const std::exception &ex) {
        std::cerr << "Error: " << ex.what() << "\n";
    }

    // Close the socket
    close(socket_fd);
    return 0;
}
