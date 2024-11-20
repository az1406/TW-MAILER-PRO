#include "functions.h"
#include <iostream>
#include <string>
#include <cstring>
#include <cctype>
#include <unistd.h>

using namespace std;

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

bool isSubjectOk(const string &subject) {
    if (subject.length() > 80) {
        cerr << "Subject may only be up to 80 characters long\n";
        return false;
    }
    return true;
}

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

string login() {
    string username, password;
    do {
        cout << "Username: ";
        getline(cin, username);
    } while (!isNameOk(username));

    password = getpass("Password: ");
    return "LOGIN\n" + username + "\n" + password;
}

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
    do {
        cout << "Message number: ";
        getline(cin, msg_number);
    } while (!isNumberOk(msg_number));

    return "READ\n" + msg_number;
}

string del() {
    string msg_number;
    do {
        cout << "Message number: ";
        getline(cin, msg_number);
    } while (!isNumberOk(msg_number));

    return "DEL\n" + msg_number;
}

bool printLogin(const string &response) {
    if (response.find("OK") == 0) {
        cout << "OK\n";
        return true;
    }
    cerr << "Login Failed: " << response << "\n";
    return false;
}

bool printList(const string &response) {
    char buffer[1024];
    strcpy(buffer, response.c_str());
    string hs = strtok(buffer, "\n");

    if (strcmp(hs.c_str(), "OK") == 0) {
        hs = strtok(NULL, "\n");  
        int messageCount = stoi(hs);

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




bool printMessage(const string &response) {
    char buffer[1024];
    strcpy(buffer, response.c_str());
    string hs = strtok(buffer, "\n");

    // If the response starts with OK, parse the message details
    if (strcmp(hs.c_str(), "OK") == 0) {
        cout << "OK" << endl;
        cout << "From: " << strtok(NULL, "\n") << "\n";  // Sender
        cout << "To: " << strtok(NULL, "\n") << "\n";    // Receiver
        cout << "Subject: " << strtok(NULL, "\n") << "\n";  // Subject
        cout << "Message: " << strtok(NULL, "") << "\n";  // Message body
        return true;
    }
    
    cerr << "Error: " << response << endl;  // Error handling
    return false;
}


bool printReply(const string &response) {
    char buffer[1024];
    strcpy(buffer, response.c_str());

    if (strcmp(buffer, "OK") == 0) {
        cout << "OK" << endl; // Ensure newline after OK
        return true;
    }
    cout << "ERR" << endl;
    cerr << "A server-side error occurred\n";
    return false;
}

