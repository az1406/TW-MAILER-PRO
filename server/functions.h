#ifndef FUNCTIONS_H
#define FUNCTIONS_H

#include <string>
#include <fstream>
#include <filesystem>
#include <iostream>  
#include <pthread.h> 
#include <signal.h>  
#include <unistd.h>  

using namespace std;


const int BUF = 1024;
const int MAX_NAME = 256;
const int MAX_SUBJ = 512;
const string SEPARATOR = ";";


extern bool abortRequested;          
extern int new_socket;               
extern int create_socket;            
extern string sender;                
extern bool successfulLogin;         
extern string clientIP;              
extern string ldapUri;               
extern int ldapVersion;              


void* clientCommunication(void* data, string folder);   
string login(string buffer, string folder);               
bool receiveFromClient(string buffer, string folder);     
string list(string folder);                               
string read(string buffer, string folder);                
bool deleteMessage(string buffer, string folder);          
void signalHandler(int sig);                              
void printUsage(void);                                   
int getNumOfFiles(string folder);                        
string getHighestFileNumber(string folder);         
string getString(string buffer);                           
string removeString(string buffer, string s1);        
bool verifyStringLength(string string, int maxStringLength); 
bool lockFile(int fd);                                 
bool unlockFile(int fd);                               

#endif // FUNCTIONS_H
