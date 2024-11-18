#include <iostream>
#include <cstring>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <fstream>
#include <string>
#include <ctime>
#include <filesystem>
#include <ldap.h>

#define PORT 8080
#define BUFFER_SIZE 1024

namespace fs = std::filesystem;

std::string getString(const std::string &buffer) {
    size_t pos = buffer.find("\n");
    if (pos != std::string::npos) {
        return buffer.substr(0, pos);  // Extract substring before newline
    }
    return buffer;  // Return the entire buffer if no newline
}

std::string removeString(std::string &buffer, const std::string &toRemove) {
    size_t pos = buffer.find(toRemove);
    if (pos != std::string::npos) {
        buffer.erase(pos, toRemove.length());  // Remove the found string
    }
    return buffer;
}

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
        int bytesReceived = recv(clientSocket, buffer, sizeof(buffer) - 1, 0);
        if (bytesReceived <= 0) {
            std::cerr << "Recv failed!" << std::endl;
            close(clientSocket);
            continue;
        }

        buffer[bytesReceived] = '\0';
        std::cout << "Received: " << buffer;

        // Check if login command
        std::string receivedMessage(buffer);
        if (receivedMessage.find("LOGIN") == 0) {
            std::cout << "Authentication requested...\n";
            std::string response = "OK\n";
            send(clientSocket, response.c_str(), response.size(), 0);
        } else {
            std::string errorResponse = "ERR\n";
            send(clientSocket, errorResponse.c_str(), errorResponse.size(), 0);
        }

        close(clientSocket);  // Close the connection
    }

    close(serverSocket);
    return 0;
}

std::string login(const std::string &buffer, const std::string &folder) {
    std::string bufferCopy = buffer;  // Create a copy for modification
    std::string username = getString(bufferCopy);
    std::string password = removeString(bufferCopy, username);

    // LDAP (authentication) processing code
    const char *ldapUri = "ldap://your-ldap-server";
    int ldapVersion = LDAP_VERSION3;

    char ldapBindUser[256];
    sprintf(ldapBindUser, "uid=%s,ou=people,dc=technikum-wien,dc=at", username.c_str());
    printf("user set to: %s\n", ldapBindUser);

    char ldapBindPassword[256];
    strcpy(ldapBindPassword, password.c_str());

    int rc = 0; /* return code */
    LDAP *ldapHandle;
    rc = ldap_initialize(&ldapHandle, ldapUri);
    if (rc != LDAP_SUCCESS) {
        fprintf(stderr, "ldap_init failed\n");
        return "ERR";
    }
    printf("Connected to LDAP server %s\n", ldapUri);

    rc = ldap_set_option(ldapHandle, LDAP_OPT_PROTOCOL_VERSION, &ldapVersion);
    if (rc != LDAP_OPT_SUCCESS) {
        fprintf(stderr, "ldap_set_option(PROTOCOL_VERSION): %s\n", ldap_err2string(rc));
        ldap_unbind_ext_s(ldapHandle, NULL, NULL);
        return "ERR";
    }

    rc = ldap_start_tls_s(ldapHandle, NULL, NULL);
    if (rc != LDAP_SUCCESS) {
        fprintf(stderr, "ldap_start_tls_s(): %s\n", ldap_err2string(rc));
        ldap_unbind_ext_s(ldapHandle, NULL, NULL);
        return "ERR";
    }

    BerValue bindCredentials;
    bindCredentials.bv_val = (char *)ldapBindPassword;
    bindCredentials.bv_len = strlen(ldapBindPassword);
    BerValue *servercredp; /* server's credentials */

    rc = ldap_sasl_bind_s(ldapHandle, ldapBindUser, LDAP_SASL_SIMPLE, &bindCredentials, NULL, NULL, &servercredp);

    ldap_unbind_ext_s(ldapHandle, NULL, NULL);

    if (rc == LDAP_SUCCESS) {
        std::cout << "Authentication successful\n";

        std::string senderFolder = folder + "/" + username;
        try {
            if (!fs::exists(senderFolder)) {
                fs::create_directory(senderFolder);
            }
        } catch (fs::filesystem_error &error) {
            std::cerr << error.what() << std::endl;
            return "ERR";
        }

        return "OK";
    }

    std::cerr << "Authentication failed\n";
    return "ERR\nInvalid credentials or LDAP error.";
}
