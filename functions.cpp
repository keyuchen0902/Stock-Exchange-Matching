#include "functions.h"
#include "tinyxml2.h"
using namespace tinyxml2;
using namespace std;
void handleRequest(int client_fd){
    pqxx::connection *C;
    try {
        // Establish a connection to the database
        // Parameters: database name, user name, user password
        C= new pqxx::connection(
            "dbname=postgres user=postgres password=passw0rd");
        if (C->is_open()) {
        // cout << "Opened database successfully: " << C->dbname() << endl;
        } else {
        cout << "Can't open database" << std::endl;
        exit(EXIT_FAILURE);
        }
    } catch (const std::exception &e) {
        cerr << e.what() << std::endl;
        exit(EXIT_FAILURE);
    }

    if (C == NULL) {
    return;
    }
    // loop and dispatch
  char buffer[40960];

  string response;

  // Reset buffer
  memset(buffer, '\0', sizeof(buffer));

  // Recv
  int totalsize = recv(client_fd, buffer, 40960, 0);
  if (totalsize < 0)
    cout << "Error receive buffer from client." << std::endl;

  string xml(buffer);
  //conver stringxml to a XMLDocument object
}