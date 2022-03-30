#include <iostream>
#include <cstring>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
#include <assert.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <stdio.h>
#include <mutex>
#include <pqxx/pqxx>
#include "database.h"
#include "functions.h"
std::mutex mymutex;
int main(int argc, char **argv)
{
  // Create socket
  int status;
  int server_fd;
  struct addrinfo host_info;
  struct addrinfo *host_info_list;
  char hostname[128];
  if (gethostname(hostname, sizeof(hostname)) == -1)
  {
    cout << "Hostname access fail" << endl;
    exit(1);
  }
  const char *port = "12345";

  memset(&host_info, 0, sizeof(host_info));

  host_info.ai_family = AF_INET;
  host_info.ai_socktype = SOCK_STREAM;

  status = getaddrinfo(hostname, port, &host_info, &host_info_list);
  if (status != 0)
  {
    cerr << "Error: cannot get address info for host" << endl;
    cerr << "  (" << hostname << "," << port << ")" << endl;
    return -1;
  } // if

  server_fd =
      socket(host_info_list->ai_family, host_info_list->ai_socktype, host_info_list->ai_protocol);
  if (server_fd == -1)
  {
    cerr << "Error: cannot create socket" << endl;
    cerr << "  (" << hostname << "," << port << ")" << endl;
    return -1;
  } // if

  int yes = 1;
  status = setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int));
  status = bind(server_fd, host_info_list->ai_addr, host_info_list->ai_addrlen);
  if (status == -1)
  {
    cerr << "Error: cannot bind socket" << endl;
    cerr << "  (" << hostname << "," << port << ")" << endl;
    return -1;
  } // if

  // Listening
  if (listen(server_fd, 100) != 0)
  {
    std::cout << "Error in listening" << std::endl;
    exit(1);
  }

  connection *C;
  try
  {
    // Establish a connection to the database
    // Parameters: database name, user name, user password
    C = new connection(
        "dbname=postgres user=postgres password=passw0rd");
    if (C->is_open())
    {
      cout << "Opened database successfully: " << C->dbname() << endl;
    }
    else
    {
      cout << "Can't open database" << endl;
      exit(EXIT_FAILURE);
    }
  }
  catch (const exception &e)
  {
    cerr << e.what() << endl;
    exit(EXIT_FAILURE);
  }

  Database::createTable(C);

  C->disconnect();
  delete C;

  while (1)
  {
    // Get address info
    struct sockaddr_storage socket_addr;
    socklen_t socket_addr_len = sizeof(socket_addr);

    // Accept request
    int client_fd = accept(server_fd, (struct sockaddr *)&socket_addr, &socket_addr_len);
    if (client_fd < 0)
    {
      cout << "Error in accept" << endl;
      exit(1);
    }

    thread onethread(handleRequest, client_fd);
    onethread.detach();
  }
  freeaddrinfo(host_info_list);
  close(server_fd);

  return 0;
}
