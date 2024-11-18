#include <iostream>
#include <stdexcept>
#include <string>
#include <unistd.h>
#include "functions.h"
#include <arpa/inet.h>

using namespace std;  // Local namespace usage for convenience

#define PORT 8080

int main(int argc, char **argv) {
    cout << "\nInitializing email client...\n\n";
    int socket_fd;
    struct sockaddr_in address{};
    bool isAuthenticated = false, isQuit = false;

    if ((socket_fd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        perror("Socket error");
        return EXIT_FAILURE;
    }

    try {
        auto address_info = get_address_info_from_command_args(argc, argv);
        address.sin_family = AF_INET;
        address.sin_port = htons(address_info.second);
        inet_aton(address_info.first.c_str(), &address.sin_addr);

        if (connect(socket_fd, (struct sockaddr *)&address, sizeof(address)) == -1) {
            perror("Connection error");
            return EXIT_FAILURE;
        }

        cout << "Connection established with " << inet_ntoa(address.sin_addr) << "\n";

        while (!isAuthenticated && !isQuit) {
            string response = handleLoginCommand(socket_fd);
            if (response == "OK") {
                isAuthenticated = true;
            } else {
                cout << "Server response: " << response << "\n";
                isQuit = getUserInput("Retry? (yes/no): ") != "yes";
            }
        }

        while (!isQuit && isAuthenticated) {
            displayMenu();
            string command = getUserInput("Enter command: ");
            string message;

            if (command == "SEND") {
                message = handleSendCommand();
            } else if (command == "LIST") {
                message = handleListCommand();
            } else if (command == "READ") {
                message = handleReadCommand();
            } else if (command == "DEL") {
                message = handleDelCommand();
            } else if (command == "QUIT") {
                message = "QUIT\n";
                isQuit = true;
            } else {
                cout << "Invalid command.\n";
                continue;
            }

            sendFormattedMessage(socket_fd, message);
            if (!isQuit) {
                string response = receiveServerResponse(socket_fd);
                cout << "Server response: " << response << "\n";
            }
        }
    } catch (const exception &ex) {
        cerr << ex.what() << "\n";
    }

    close(socket_fd);
    return EXIT_SUCCESS;
}
