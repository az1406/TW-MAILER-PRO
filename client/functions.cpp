#include "functions.h"
#include <iostream>
#include <regex>
#include <stdexcept>
#include <vector>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>


using namespace std;  // Local namespace usage to reduce repetition in this source file

std::string getUserInput(const std::string &prompt) {
    cout << prompt;
    string input;
    getline(cin, input);
    return input;
}

bool is_valid_ip_address(const std::string &address) {
    const regex ip_regex(R"(^(\d{1,3})\.(\d{1,3})\.(\d{1,3})\.(\d{1,3})$)");
    smatch match;
    if (regex_match(address, match, ip_regex)) {
        for (size_t i = 1; i <= 4; ++i) {
            int octet = stoi(match[i].str());
            if (octet < 0 || octet > 255) return false;
        }
        return true;
    }
    return false;
}

bool is_valid_port(int port) {
    return port > 1023 && port <= 65535;
}

std::pair<std::string, int> get_address_info_from_command_args(int argc, char **argv) {
    if (argc < 3) throw invalid_argument("Provide IP address and port.");
    string ip_address = argv[1];
    if (!is_valid_ip_address(ip_address)) throw invalid_argument("Invalid IP address format.");
    int port = stoi(argv[2]);
    if (!is_valid_port(port)) throw invalid_argument("Port must be between 1024 and 65535.");
    return {ip_address, port};
}

void sendFormattedMessage(int socket, const string &message) {
    int messageLength = static_cast<int>(message.size());
    send(socket, reinterpret_cast<const char *>(&messageLength), sizeof(messageLength), 0);
    send(socket, message.c_str(), message.length(), 0);
}

std::string receiveServerResponse(int socket) {
    int message_length;
    recv(socket, &message_length, sizeof(message_length), 0);
    vector<char> buffer(message_length + 1, '\0');
    recv(socket, buffer.data(), message_length, 0);
    return string(buffer.data());
}

std::string handleLoginCommand(int socket) {
    string username = getUserInput("Enter LDAP username: ");
    string password = getUserInput("Enter password: ");
    string loginMessage = "LOGIN\n" + username + "\n" + password + "\n";
    sendFormattedMessage(socket, loginMessage);
    return receiveServerResponse(socket);
}

std::string handleSendCommand() {
    string receiver = getUserInput("Enter Receiver: ");
    string subject = getUserInput("Enter Subject: ");
    cout << "Enter Message (end with '.'): ";
    string line, message;
    while (getline(cin, line) && line != ".") {
        message += line + "\n";
    }
    return "SEND\n" + receiver + "\n" + subject + "\n" + message + ".\n";
}

std::string handleListCommand() {
    return "LIST\n";
}

std::string handleReadCommand() {
    string msgNum = getUserInput("Enter Message Number: ");
    return "READ\n" + msgNum + "\n";
}

std::string handleDelCommand() {
    string msgNum = getUserInput("Enter Message Number: ");
    return "DEL\n" + msgNum + "\n";
}

void displayMenu() {
    cout << "\nAvailable Commands:\n";
    cout << "SEND  -> Send a new message\n";
    cout << "LIST  -> List all messages\n";
    cout << "READ  -> Read a message\n";
    cout << "DEL   -> Delete a message\n";
    cout << "QUIT  -> Quit the application\n";
}
