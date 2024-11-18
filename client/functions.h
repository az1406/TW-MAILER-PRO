#ifndef FUNCTIONS_H
#define FUNCTIONS_H

#include <string>

std::string getUserInput(const std::string &prompt);
std::pair<std::string, int> get_address_info_from_command_args(int argc, char **argv);
void sendFormattedMessage(int socket, const std::string &message);
std::string receiveServerResponse(int socket);
std::string handleLoginCommand(int socket);

#endif // FUNCTIONS_H
