#include <iostream>
#include <cstring>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <ldap.h>  // Correctly include LDAP library

#define PORT 8080
#define BUFFER_SIZE 1024

// Function to authenticate user via LDAP
bool authenticateWithLDAP(const std::string &username, const std::string &password) {
    LDAP *ldap_handle;
    LDAPMessage *result;
    LDAPMessage *entry;
    BerElement *ber;
    char *dn;
    
    // Initialize LDAP connection
    ldap_initialize(&ldap_handle, "ldap://localhost"); // Adjust your LDAP server address
    if (ldap_handle == NULL) {
        std::cerr << "LDAP initialization failed!" << std::endl;
        return false;
    }
    
    // Bind to LDAP server with the provided username and password
    int version = LDAP_VERSION3;
    ldap_set_option(ldap_handle, LDAP_OPT_PROTOCOL_VERSION, &version);
    int bind_result = ldap_simple_bind_s(ldap_handle, username.c_str(), password.c_str());

    if (bind_result == LDAP_SUCCESS) {
        // Search for the user in the LDAP directory (optional, you can skip if bind succeeds)
        std::string base_dn = "ou=users,dc=example,dc=com"; // Change according to your LDAP structure
        std::string filter = "(uid=" + username + ")";

        int search_result = ldap_search_ext_s(ldap_handle, base_dn.c_str(), LDAP_SCOPE_SUBTREE, filter.c_str(), NULL, 0, NULL, NULL, NULL, 0, &result);
        if (search_result == LDAP_SUCCESS) {
            entry = ldap_first_entry(ldap_handle, result);
            if (entry != NULL) {
                dn = ldap_get_dn(ldap_handle, entry);
                std::cout << "Authenticated user: " << dn << std::endl;
                ldap_memfree(dn);
                ldap_msgfree(result);
                ldap_unbind(ldap_handle);
                return true; // Authentication successful
            }
        }
    }
    
    // If bind failed or search didn’t find user, return false
    ldap_unbind(ldap_handle);
    return false;
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
            
            // Parse the login details
            size_t username_pos = receivedMessage.find("\n") + 1;
            size_t password_pos = receivedMessage.find("\n", username_pos) + 1;
            std::string username = receivedMessage.substr(username_pos, receivedMessage.find("\n", username_pos) - username_pos);
            std::string password = receivedMessage.substr(password_pos);

            // Authenticate the user
            if (authenticateWithLDAP(username, password)) {
                std::string response = "OK\n";
                send(clientSocket, response.c_str(), response.size(), 0);
            } else {
                std::string errorResponse = "ERR\n";
                send(clientSocket, errorResponse.c_str(), errorResponse.size(), 0);
            }
        } else {
            std::string errorResponse = "ERR\n";
            send(clientSocket, errorResponse.c_str(), errorResponse.size(), 0);
        }

        close(clientSocket);  // Close the connection
    }

    close(serverSocket);
    return 0;
}
