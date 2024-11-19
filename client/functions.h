#ifndef FUNCTIONS_H
#define FUNCTIONS_H

#include <string>
using namespace std;

bool isNameOk(const string& name);
bool isSubjectOk(const string& subject);
bool isMessageOk(const string& message);
bool isNumberOk(const string& number);
string login();
string send();
string list();
string read();
string del();
bool printLogin(const string& response);
bool printList(const string& response);
bool printMessage(const string& response);
bool printReply(const string& response);

#endif
