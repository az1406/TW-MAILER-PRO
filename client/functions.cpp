#include "functions.h"
#include <iostream>
#include <regex>
#include <stdexcept>
#include <vector>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <cstring>

std::string getUserInput(const std::string &prompt) {
    std::cout << prompt;
    std::string input;
    std::getline(std::cin, input);
    return input;
}

std::pair<std::string, int> get_address_info_from_command_args(int argc, char **argv) {
    if (argc < 3) throw std::invalid_argument("Provide IP address and port.");
    std::string ip_address = argv[1];
    int port = std::stoi(argv[2]);
    return {ip_address, port};
}

void sendFormattedMessage(int socket, const std::string &message) {
    if (send(socket, message.c_str(), message.size(), 0) == -1) {
        perror("Send error");
    }
}

std::string receiveServerResponse(int socket) {
    char buffer[1024] = {0};
    int bytes_received = recv(socket, buffer, sizeof(buffer), 0);
    if (bytes_received > 0) {
        return std::string(buffer, bytes_received);
    }
    return "";
}

std::string handleLoginCommand(int socket) {
    std::string username = getUserInput("Enter LDAP username: ");
    std::string password = getUserInput("Enter password: ");
    std::string loginMessage = "LOGIN\n" + username + "\n" + password + "\n";
    sendFormattedMessage(socket, loginMessage);
    return receiveServerResponse(socket);
}
