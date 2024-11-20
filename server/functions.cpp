#include <sys/types.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <sys/file.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string>
#include <signal.h>
#include <string.h>
#include <iostream>
#include <filesystem>
#include <fstream>
#include <ldap.h>
using namespace std;
namespace fs = std::filesystem;

#define BUF 1024

#define SEPERATOR '\n'
#define MAX_NAME 8
#define MAX_SUBJ 80

const char *ldapUri = "ldap://ldap.technikum-wien.at:389";
const int ldapVersion = LDAP_VERSION3;
bool successfulLogin = false;
int abortRequested = 0;
int create_socket = -1;
int new_socket = -1;
string sender = "";
string clientIP = "";
pid_t childpid;

string removeString(string buffer, string s1)
{
   return buffer.erase(0, s1.length() + 1);
}

string getString(string buffer)
{
   string helper;
   size_t pos = buffer.find(SEPERATOR);
   helper = buffer.substr(0, pos);
   return helper;
}


int getNumOfFiles(string folder)
{
   int count = 0;
   try {
      fs::path path = folder;

      for (auto& p : fs::directory_iterator(path)) {
         count++;
      }
   } catch (fs::filesystem_error& error) {
      cerr << error.what() << endl;
      exit(EXIT_FAILURE);
   }
   return count; /* - 1 because numOfFiles.txt doesn't count to the number of messages */
}

bool verifyStringLength(string string, int maxStringLength)
{
   return (string.length() <= (unsigned)maxStringLength);
}

bool deleteMessage(string buffer, string folder)
{
   string messageNumber;
   messageNumber = buffer;
   string usernameFolder = folder + "/" + sender;
   string searchedFileDirectory;
   int counter = 0;
   for (fs::directory_entry e: fs::directory_iterator(usernameFolder)){
      counter++;
      if(counter > stoi(buffer)){
         return "ERR";
      }
      if(counter == stoi(buffer)){
         searchedFileDirectory = e.path();
         break;
      }
   }

   try {
      if(!fs::exists(searchedFileDirectory)) {
         return false;
      }
      fs::remove(searchedFileDirectory);
   } catch (fs::filesystem_error& error) {
      cerr << error.what() << endl;
      return false;
   }
   return true;
}

string list(string folder)
{
   if(!verifyStringLength(sender, MAX_NAME)) {
      return "ERR";
   }
   string userFolder = folder + "/" + sender; /* get username's folder */
   
   try {
      if(!fs::exists(userFolder)){ /* username doesn't have a folder -> return 0 */
        cout << userFolder << "does not exist" << endl;
         return to_string(0);
      }
   } catch (fs::filesystem_error& error) {
      cerr << error.what() << endl;
      return "ERR";
   }
   const fs::path path = userFolder; 
   string helperString; /* return string */

   helperString += to_string(getNumOfFiles(userFolder));

   try{
      for (const auto& entry : fs::directory_iterator(path)) {
         const auto filenameStr = entry.path().filename().string(); /* get name of file */
         helperString += SEPERATOR;
         
         /* open the file and get the subject */
         string line;
         string subject; 
         ifstream file(userFolder + "/" + filenameStr);
         int counter = 0;

         if(file.is_open()) {
            while(getline(file, line)){
               ++counter;
               if(counter == 3) { /* subject is in the third line of every file */
                  subject = line;
               }
            }
         } else {
            cerr << strerror(errno);
            cout << "file.is_open() error" << endl;
            return "ERR";
         }
         file.close();
         helperString += subject;

      }
   } catch (fs::filesystem_error& error) {
      cerr << error.what() << endl;
      return "ERR";
   }
   cout << "Helper string: " << helperString << endl;
   string returnString = "OK\n";
   returnString = returnString + helperString;
   cout << returnString << endl;
   return returnString;
}

string read(string buffer, string folder)
{
   string messageNumber;

   messageNumber = buffer;
   cout << "read, message number: " << messageNumber << endl;
   string usernameFolder = folder + "/" + sender;
   string searchedFileDirectory;
   int counter = 0;
   for (fs::directory_entry e: fs::directory_iterator(usernameFolder)){
      counter++;
      if(counter > stoi(buffer)){
         return "ERR";
      }
      if(counter == stoi(buffer)){
         searchedFileDirectory = e.path();
         break;
      }
   }

   try{
      if(!fs::exists(searchedFileDirectory)){
         cout << "file does not exist" << endl;
         return "ERR";
      }
   } catch (fs::filesystem_error& error) {
      cerr << error.what() << endl;
      return "ERR";
   }

   string message; 
   string line;

 ifstream file(searchedFileDirectory); 
   if(file.is_open()) {
      while(!file.eof()) { 
         getline(file, line);
         message += line;
         message += "\n";
      }
   } else {
      cerr << "couldn't open file" << endl;
      return "ERR";
   }
   file.close();
   
   return "OK\n" + message;

}

string getHighestFileNumber(string folder)
{
   fs::directory_entry e;
   for(int i = 1; ; i++){
      e = fs::directory_entry(folder + "/" + to_string(i) + ".txt");
      if(!e.exists()){
         return to_string(i);
      }

   }
}

bool receiveFromClient(string buffer, string folder){

   string receiver, subject, message;
      
   cout << "buffer: " << buffer << endl;
   receiver = getString(buffer); /* get receiver */
   if(!verifyStringLength(receiver, MAX_NAME)) {
      return false;
   }
   buffer = removeString(buffer, receiver);
   
   subject = getString(buffer); 
   if(!verifyStringLength(subject, MAX_SUBJ)) {
      return false;
   }
   buffer = removeString(buffer, subject);
   message = buffer; 
   cout << "Receiver: " << receiver << endl << "Subject: " << subject << endl << "Message: " << message << endl;
   string receiverFolder = folder + "/" + receiver;
   try {
     if(!fs::exists(receiverFolder)) { 
         fs::create_directory(receiverFolder);
      }
   } catch (fs::filesystem_error& error) {
      cerr << error.what() << endl;
      return false;
   }

   
   ofstream outfile; 
   string newFile = ""; 
   string fileNumber = ""; 
   
   
   fileNumber += getHighestFileNumber(receiverFolder);
   if(strcasecmp(fileNumber.c_str(), "ERR") == 0) {
      return false;
   }
   newFile += receiverFolder + "/" + fileNumber + ".txt";

   outfile.open(newFile.c_str()); 
   if(!outfile){
      cerr << "newFile couldn't be opened" << endl;
      return false;
   }
   outfile << sender << endl << receiver << endl << subject << endl << message;
   outfile.close(); 

   return true;
}

string login(string buffer, string folder) {
    char buff[1024];
    strcpy(buff, buffer.c_str());
    string username = getString(buffer);  
    string password = removeString(buffer, username); 

    if (clientIP.empty()) {
        cerr << "Couldn't access the client IP address" << endl;
        return "ERR";
    }

    time_t now = time(0);

    fstream blacklist;
    string line;
    bool isBlacklisted = false;

    blacklist.open("blacklist.txt", ios::in);
    if (blacklist.is_open()) {
        while (getline(blacklist, line)) {
            string ip, timeStr;
            string delimiter = ";";

            size_t pos = 0;
            pos = line.find(delimiter);
            ip = line.substr(0, pos);
            line.erase(0, pos + delimiter.length());
            timeStr = line;

            if (clientIP == ip && (now - stoi(timeStr) < 60)) {
                isBlacklisted = true;
                break;
            }
        }
        blacklist.close();
    }

    if (isBlacklisted) {
        return "ERR\nYour IP is blacklisted. Please try again later.";
    }

    char ldapBindUser[256];
    sprintf(ldapBindUser, "uid=%s,ou=people,dc=technikum-wien,dc=at", username.c_str());

    char ldapBindPassword[256];
    strcpy(ldapBindPassword, password.c_str());

    int rc = 0;
    LDAP *ldapHandle;
    rc = ldap_initialize(&ldapHandle, ldapUri);
    if (rc != LDAP_SUCCESS) {
        fprintf(stderr, "ldap_init failed\n");
        return "ERR";
    }

    rc = ldap_set_option(ldapHandle, LDAP_OPT_PROTOCOL_VERSION, &ldapVersion);
    if(rc != LDAP_OPT_SUCCESS) {
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
    BerValue *servercredp;

    rc = ldap_sasl_bind_s(ldapHandle, ldapBindUser, LDAP_SASL_SIMPLE, &bindCredentials, NULL, NULL, &servercredp);

    ldap_unbind_ext_s(ldapHandle, NULL, NULL);

    if(rc == LDAP_SUCCESS) {
        sender = username;
        successfulLogin = true;

        
        string senderFolder = folder + "/" + sender;
        if (!fs::exists(senderFolder)) {
            fs::create_directory(senderFolder);
        }
        return "OK";
    }

    fstream loginLogFile;
    loginLogFile.open("loginLog.txt", ios_base::app);
    if (!loginLogFile) {
        cerr << "loginLog.txt couldn't be opened" << endl;
        return "ERR";
    }

    loginLogFile << clientIP << ";" << username << ";" << now << endl;
    loginLogFile.close();

    int attemptCounter = 0;

    loginLogFile.open("loginLog.txt", ios::in);
    if (loginLogFile.is_open()) {
        while (getline(loginLogFile, line)) {
            string ip, user, timeStr;
            string delimiter = ";";

            size_t pos = 0;
            
            pos = line.find(delimiter);
            ip = line.substr(0, pos);
            line.erase(0, pos + delimiter.length());

            pos = line.find(delimiter);
            user = line.substr(0, pos);
            line.erase(0, pos + delimiter.length());

            timeStr = line;

            
            if (clientIP == ip && (now - stoi(timeStr) < 60)) {
                attemptCounter++;
            }
        }
        loginLogFile.close();
    }

    
    if (attemptCounter >= 3) {
        fstream blacklistFile;
        blacklistFile.open("blacklist.txt", ios_base::app);
        if (!blacklistFile) {
            cerr << "blacklist.txt couldn't be opened, creating a new file" << endl;
            blacklistFile.open("blacklist.txt", ios_base::out);  
        }
        blacklistFile << clientIP << ";" << now << endl;
        blacklistFile.close();
        return "ERR\nYour IP is blacklisted. Please try again later.";
    }

    return "ERR";
}

bool lockFile(int fd) {
   if(flock(fd, LOCK_SH | LOCK_NB)) {
      if(errno == EWOULDBLOCK) { 
         if(flock(fd, LOCK_SH) == -1 ){
            cerr << "Failed to lock the file " << endl;
            close(fd);
            return false;
         }
      } else {
         cerr << "Failed to lock the file " << endl;
         close(fd);
         return false;
      }
   }
   return true;
}

bool unlockFile(int fd) {
   if(flock(fd, LOCK_UN) == -1) {
      cerr << "Failed to unlock the file!" << endl;
      close(fd);
      return false;
   }
   close(fd);
   return true;
}

void *clientCommunication(void *data, string folder)
{
   char buffer[BUF];
   int size;
   int *current_socket = (int *)data;

   strcpy(buffer, "Welcome to the server!\r\nPlease enter your commands...\r\n");
   if (send(*current_socket, buffer, strlen(buffer), 0) == -1) {
      perror("send failed");
      return NULL;
   }

   do {
      size = recv(*current_socket, buffer, BUF - 1, 0);
      if (size == -1) {
         if (abortRequested) {
            perror("recv error after aborted");
         }
         else {
            perror("recv error");
         }
         break;
      }

      if (size == 0) {
         printf("Client closed remote socket\n");
         break;
      }

      if (buffer[size - 2] == '\r' && buffer[size - 1] == '\n') {
         size -= 2;
      }
      else if (buffer[size - 1] == '\n') {
         --size;
      }

      buffer[size] = '\0';
      string flag = getString(buffer);
      string bufferString = removeString(buffer, flag);
      string response = "";

      if(strcasecmp(flag.c_str(), "LOGIN") == 0) {
         response = login(bufferString, folder);
      }

      if(successfulLogin) {
         if (strcasecmp(flag.c_str(), "SEND") == 0) {
            response = receiveFromClient(bufferString, folder) ? "OK" : "ERR";
         } 
         else if (strcasecmp(flag.c_str(), "LIST") == 0) {
            response = list(folder);
         }
         else if (strcasecmp(flag.c_str(), "READ") == 0) {
            response = read(bufferString, folder);
         }
         else if (strcasecmp(flag.c_str(), "DEL") == 0) {
            response = deleteMessage(bufferString, folder) ? "OK" : "ERR";
         } 
      }
      
      if (send(*current_socket, response.c_str(), strlen(response.c_str()), 0) == -1)
      {
         perror("send answer failed");
         return NULL;
      }
      
} while (strcmp(buffer, "quit") != 0 && !abortRequested);

   if (*current_socket != -1) {
      if (shutdown(*current_socket, SHUT_RDWR) == -1) {
         perror("shutdown new_socket");
      }
      if (close(*current_socket) == -1) {
         perror("close new_socket");
      }
      *current_socket = -1;
   }

   return NULL;
}
