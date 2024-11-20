#include "functions.h"
#include <iostream>
#include <string>
#include <cstring>
#include <cctype>
#include <unistd.h>

using namespace std;
// Validate that the provided username
bool isNameOk(const string &name) {
    if (name.length() > 8) {
        cerr << "Given name is too long\n";
        return false;
    }
    if (name.empty()) {
        cerr << "Entered empty name\n";
        return false;
    }
    for (char c : name) {
        if (!isalnum(c)) {
            cerr << "Given name contains illegal characters\n";
            return false;
        }
    }
    return true;
}
// Validate the length of the email subject
bool isSubjectOk(const string &subject) {
    if (subject.length() > 80) {
        cerr << "Subject may only be up to 80 characters long\n";
        return false;
    }
    return true;
}
// Validate the length of the email message
bool isMessageOk(const string &message) {
    if (message.length() > 927) {
        cerr << "Message may only be up to 927 characters long\n";
        return false;
    }
    return true;
}

bool isNumberOk(const string &number) {
    if (number.empty()) {
        cerr << "No number was entered\n";
        return false;
    }
    for (char c : number) {
        if (!isdigit(c)) {
            cerr << "Invalid input: contains non-digit characters\n";
            return false;
        }
    }
    return true;
}
// Construct the LOGIN command with user credentials
string login() {
    string username, password;
    do {
        cout << "Username: ";
        getline(cin, username);
    } while (!isNameOk(username));

    password = getpass("Password: ");// Securely prompt for the password
    return "LOGIN\n" + username + "\n" + password;
}
// Construct the SEND command with recipient, subject, and message
string sendMail() {
    string receiver, subject, message, hs;
    do {
        cout << "Receiver: ";
        getline(cin, receiver);
    } while (!isNameOk(receiver));

    do {
        cout << "Subject: ";
        getline(cin, subject);
    } while (!isSubjectOk(subject));

    do {
        cout << "Message (end with '.' on a new line):\n";
        message.clear();
        while (getline(cin, hs) && hs != ".") {
            message += hs + "\n";
        }
    } while (!isMessageOk(message));

    return "SEND\n" + receiver + "\n" + subject + "\n" + message;
}

string list() {
    return "LIST\n";
}

string read() {
    string msg_number;
    do { //Validate the message number
        cout << "Message number: ";
        getline(cin, msg_number);
    } while (!isNumberOk(msg_number));

    return "READ\n" + msg_number;
}
// Construct the DELETE command
string del() {
    string msg_number;
    do {
        cout << "Message number: ";
        getline(cin, msg_number);
    } while (!isNumberOk(msg_number));

    return "DEL\n" + msg_number;
}
// Handle and print the server response for the LOGIN command
bool printLogin(const string &response) {
    if (response.find("OK") == 0) {
        cout << "OK\n";
        return true;
    }
    cerr << "Login Failed: " << response << "\n";
    return false;
}
// Parse and display the response from the LIST command
bool printList(const string &response) {
    char buffer[1024];
    strcpy(buffer, response.c_str());
    string hs = strtok(buffer, "\n");

    if (strcmp(hs.c_str(), "OK") == 0) {
        hs = strtok(NULL, "\n");  
        int messageCount = stoi(hs);
// Loop through and print each message subject
        for (int i = 1; i <= messageCount; i++) {
            char *message = strtok(NULL, "\n");  
            if (message != nullptr) {
                cout << i << ". " << message << endl;  
            }
        }
        return true;
    }
    cerr << "A server-side error occurred\n";
    return false;
}

// Parse and display the response from the READ command
bool printMessage(const string &response) {
    char buffer[1024];
    strcpy(buffer, response.c_str());
    string hs = strtok(buffer, "\n");

    // If the response starts with OK, parse the message details
    if (strcmp(hs.c_str(), "OK") == 0) {
        cout << "OK" << endl;
        cout << "From: " << strtok(NULL, "\n") << "\n";  // Sender...
        cout << "To: " << strtok(NULL, "\n") << "\n";    
        cout << "Subject: " << strtok(NULL, "\n") << "\n";  
        cout << "Message: " << strtok(NULL, "") << "\n"; 
        return true;
    }
    
    cerr << "Error: " << response << endl; 
    return false;
}

// Handle and print the response 
bool printReply(const string &response) {
    char buffer[1024];
    strcpy(buffer, response.c_str());

    if (strcmp(buffer, "OK") == 0) {
        cout << "OK" << endl;
        return true;
    }
    cout << "ERR" << endl;
    cerr << "A server-side error occurred\n";
    return false;
}

