#ifndef FUNCTIONS_H
#define FUNCTIONS_H

#include <string>

std::string getUserInput(const std::string &prompt);
bool is_valid_ip_address(const std::string &address);
bool is_valid_port(int port);
std::pair<std::string, int> get_address_info_from_command_args(int argc, char **argv);
void sendFormattedMessage(int socket, const std::string &message);
std::string receiveServerResponse(int socket);
std::string handleLoginCommand(int socket);
std::string handleSendCommand();
std::string handleListCommand();
std::string handleReadCommand();
std::string handleDelCommand();
void displayMenu();

#endif // FUNCTIONS_H
