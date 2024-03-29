#ifndef FUNCTIONS_H
#define FUNCTIONS_H

#include <netdb.h>
#include <netinet/in.h>
#include <sys/select.h>
#include <unistd.h>

#include <chrono>
#include <cstring>
#include <exception>
#include <iostream>
#include <mutex>
#include <pqxx/pqxx>
#include <string>
#include <thread>

#include "database.h"
#include "handledb.h"

using namespace tinyxml2;
using namespace std;


void handleRequest(int client_fd);  // parse
XMLDocument* handleCreat(connection *C, string &request);
XMLDocument* handleTranscation(connection *C, string &request);
long getCurrTime();
#endif