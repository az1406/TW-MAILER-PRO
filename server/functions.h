#ifndef FUNCTIONS_H
#define FUNCTIONS_H

#include <string>
#include <fstream>
#include <filesystem>
#include <iostream>  // For handling output (such as errors or debug info)
#include <pthread.h> // For multi-threading support if needed
#include <signal.h>  // For signal handling
#include <unistd.h>  // For standard system calls (if needed)

using namespace std;

// Define constants
const int BUF = 1024;
const int MAX_NAME = 256;
const int MAX_SUBJ = 512;
const string SEPARATOR = ";";

// Declare global variables
extern bool abortRequested;          // Ensure this is defined in one of your source files
extern int new_socket;               // Ensure this is defined
extern int create_socket;            // Ensure this is defined
extern string sender;                // Ensure this is defined
extern bool successfulLogin;         // Ensure this is defined
extern string clientIP;              // Ensure this is defined
extern string ldapUri;               // Ensure this is defined
extern int ldapVersion;              // Ensure this is defined

// Function Declarations
void* clientCommunication(void* data, string folder);      // Thread function for handling client communication
string login(string buffer, string folder);                // Handle login process
bool receiveFromClient(string buffer, string folder);     // Receive data from client
string list(string folder);                               // List files or messages in folder
string read(string buffer, string folder);                // Read message or file
bool deleteMessage(string buffer, string folder);         // Delete message/file
void signalHandler(int sig);                              // Handle signals
void printUsage(void);                                    // Print usage information
int getNumOfFiles(string folder);                         // Get number of files in folder
string getHighestFileNumber(string folder);               // Get the highest file number in folder
string getString(string buffer);                          // Get string data from buffer
string removeString(string buffer, string s1);            // Remove string s1 from buffer
bool verifyStringLength(string string, int maxStringLength); // Verify string length
bool lockFile(int fd);                                    // Lock a file
bool unlockFile(int fd);                                  // Unlock a file

#endif // FUNCTIONS_H
