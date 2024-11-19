#include "functions.h"
#include <iostream>
#include <string>
#include <cstring>
#include <cctype>
#include <unistd.h>

// Use the `std` namespace to avoid prefixing with std::
using namespace std;

bool isNameOk(const string& name) {
    if (name.length() > 8) {
        cout << "Given name is too long\n";
        return false;
    }
    if (name.empty()) {
        cout << "Entered empty name\n";
        return false;
    }

    for (char c : name) {
        if (!isalnum(c)) {
            cout << "Given name contains illegal characters\n";
            return false;
        }
    }
    return true;
}

bool isSubjectOk(const string& subject) {
    if (subject.length() > 80) {
        cout << "Subject may only be up to 80 characters long\n";
        return false;
    }
    return true;
}

bool isMessageOk(const string& message) {
    if (message.length() > 927) {
        cout << "Message may only be up to 927 characters long\n";
        return false;
    }
    return true;
}

bool isNumberOk(const string& number) {
    if (number.empty()) {
        cout << "No number was put in\n";
        return false;
    }
    for (char c : number) {
        if (!isdigit(c)) {
            cout << "Given string contains non-digit characters\n";
            return false;
        }
    }
    return true;
}

string login() {
    string username, password, package;

    do {
        cout << "Username: ";
        getline(cin, username);
    } while (!isNameOk(username));

    password = getpass("Password: ");
    package = "LOGIN\n" + username + '\n' + password;
    
    return package;
}

string send() {
    string receiver, subject, message, hs, package;
    do {
        cout << "Receiver: ";
        getline(cin, receiver);
    } while (!isNameOk(receiver));
    
    do {
        cout << "Subject: ";
        getline(cin, subject);
    } while (!isSubjectOk(subject));
    
    do {
        cout << "Message:\n";
        do {
            hs = "";
            getline(cin, hs);
            if (hs != ".") {
                message += hs;
                message += "\n";
            }
        } while (hs != ".");
    } while (!isMessageOk(message));

    package = "SEND\n" + receiver + "\n" + subject + "\n" + message;
    return package;
}

string list() {
    return "LIST\n";
}

string read() {
    string msg_number, package;
    do {
        cout << "Message number: ";
        getline(cin, msg_number);
    } while (!isNumberOk(msg_number));

    package = "READ\n" + msg_number;
    return package;
}

string del() {
    string package, msg_number;
    do {
        cout << "Message number: ";
        getline(cin, msg_number);
    } while (!isNumberOk(msg_number));

    package = "DEL\n" + msg_number;
    return package;
}

bool printLogin(const string& response) {
    char buffer[1024];
    strcpy(buffer, response.c_str());
    string hs = strtok(buffer, ";");

    if (strcmp(hs.c_str(), "OK") == 0) {
        cout << "<< Login Successful" << endl;
        return true;
    } else if (strcmp(hs.c_str(), "ERR") == 0) {
        cerr << "A server-side error occurred\n";
        return false;
    }

    cout << buffer << endl;
    return true;
}

bool printList(const string& response) {
    char buffer[1024];
    strcpy(buffer, response.c_str());
    string hs = strtok(buffer, "\n");

    if (strcmp(hs.c_str(), "OK") == 0) {
        hs = strtok(NULL, "\n");
        cout << "<< " << hs << endl;
        for (int i = 1; i <= stoi(hs); i++) {
            printf("<< Message number: %i | Subject: %s\n", i, strtok(NULL, "\n"));
        }
        return true;
    }
    cerr << "A server-side error occurred\n";
    return false;
}

bool printMessage(const string& response) {
    char buffer[1024];
    strcpy(buffer, response.c_str());
    string hs = strtok(buffer, "\n");
    cout << "<< " << hs << "\n";

    if (strcmp(hs.c_str(), "OK") == 0) {
        cout << "Sender: " << strtok(NULL, "\n") << "\n";
        cout << "Receiver: " << strtok(NULL, "\n") << "\n";
        cout << "Subject: " << strtok(NULL, "\n") << "\n";
        cout << "Message: " << strtok(NULL, "") << "\n";
        return true;
    }
    cerr << "A server-side error occurred\n";
    return false;
}

bool printReply(const string& response) {
    char buffer[1024];
    strcpy(buffer, response.c_str());

    if (strcmp(buffer, "OK") == 0) {
        cout << "<< OK" << endl;
        return true;
    }
    cout << "<< ERR" << endl;
    cerr << "A server-side error occurred\n";
    return false;
}
