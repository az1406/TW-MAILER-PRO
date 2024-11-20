#include <iostream>
#include <csignal>
#include <cstring>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <cstdlib>
#include <filesystem>
#include "functions.h"

using namespace std;
namespace fs = std::filesystem;
int main(int argc, char* argv[]) {
   
   int childpid;
   if (argc != 3) {
      printUsage();
   }

   string folder = argv[2];
// Try to create the directory for storing emails if it doesn't exist
   try {
      if (!fs::is_directory(folder)) {
         cout << folder << " does not exist. Creating now..." << endl;
         fs::create_directory(folder);
      }
   } catch(fs::filesystem_error& error) {
      cerr << error.what() << endl;
      exit(EXIT_FAILURE);
   }

   socklen_t addrlen;
   struct sockaddr_in address, cliaddress;
   int reuseValue = 1;

// Handle the interrupt signal (SIGINT) for shutdown
   if (signal(SIGINT, signalHandler) == SIG_ERR) {
      perror("signal can not be registered");
      return EXIT_FAILURE;
   }
// Create the server socket using IPv4 and TCP (SOCK_STREAM)
   if ((create_socket = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
      perror("Socket error");
      return EXIT_FAILURE;
   }
// Set socket options for address and port reuse
   if (setsockopt(create_socket, SOL_SOCKET, SO_REUSEADDR, &reuseValue, sizeof(reuseValue)) == -1) {
      perror("set socket options - reuseAddr");
      return EXIT_FAILURE;
   }
// Zero out the memory for the server address structure and set values
   if (setsockopt(create_socket, SOL_SOCKET, SO_REUSEPORT, &reuseValue, sizeof(reuseValue)) == -1) {
      perror("set socket options - reusePort");
      return EXIT_FAILURE;
   }

   memset(&address, 0, sizeof(address));
   address.sin_family = AF_INET;
   address.sin_addr.s_addr = INADDR_ANY;
// Validate the provided port number to ensure it is within the valid range
   try {
      if(stoi(argv[1]) < 1024 || stoi(argv[1]) > 65535) {
         cerr << "Input Port is not in usable port range" << endl;
         exit(EXIT_FAILURE);
      }
   } catch (invalid_argument& e1) {
      cerr << "Port was not a number" << endl;
      exit(EXIT_FAILURE);
   }

   address.sin_port = htons(stoi(argv[1]));

   if (bind(create_socket, (struct sockaddr *)&address, sizeof(address)) == -1) {
      perror("bind error");
      return EXIT_FAILURE;
   }

   if (listen(create_socket, 5) == -1) {
      perror("listen error");
      return EXIT_FAILURE;
   }
// Main loop: server waits for incoming client connections
   while (!abortRequested) {
      printf("Waiting for connections...\n");

      addrlen = sizeof(struct sockaddr_in);
      if ((new_socket = accept(create_socket, (struct sockaddr *)&cliaddress, &addrlen)) == -1) {
         if (abortRequested) {
            perror("accept error after aborted");
         } else {
            perror("accept error");
         }
         break;
      }

      printf("Client connected from %s:%d...\n",
             inet_ntoa(cliaddress.sin_addr),
             ntohs(cliaddress.sin_port));

      if ((childpid = fork()) == 0) {
         close(create_socket);

         clientIP = inet_ntoa(cliaddress.sin_addr);
         clientCommunication(&new_socket, folder);

         close(new_socket);
         exit(EXIT_SUCCESS);
      }

      close(new_socket);
   }
// Create a child process for handling the client's request
   while ((childpid = waitpid(-1, NULL, WNOHANG))) {
      if ((childpid == -1) && (errno != EINTR)) {
         break;
      }
   }

   if (create_socket != -1) {
      if (shutdown(create_socket, SHUT_RDWR) == -1) {
         perror("shutdown create_socket");
      }
      if (close(create_socket) == -1) {
         perror("close create_socket");
      }
      create_socket = -1;
   }

   return EXIT_SUCCESS;
}

// Signal handler function for SIGINT to perform shutdown
void signalHandler(int sig) {
    if (sig == SIGINT) {
        printf("abort Requested... \n");
        abortRequested = 1;
         // Close the client socket if it is open
        if (new_socket != -1) {
            if (shutdown(new_socket, SHUT_RDWR) == -1) {
                perror("shutdown new_socket");
            }
            if (close(new_socket) == -1) {
                perror("close new_socket");
            }
            new_socket = -1;
        }
// Shutdown and close the server socket if it's still ope
        if (create_socket != -1) {
            if (shutdown(create_socket, SHUT_RDWR) == -1) {
                perror("shutdown create_socket");
            }
            if (close(create_socket) == -1) {
                perror("close create_socket");
            }
            create_socket = -1;
        }
    } else {
        exit(sig);
    }
}
void printUsage(void) {
    printf("Incorrect usage. Start the server using: \"./twmailer-server <port> <mail-spool-directoryname>\"\n");
    exit(EXIT_FAILURE);
}
