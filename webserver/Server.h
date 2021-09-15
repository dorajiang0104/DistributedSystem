/**
 * Server.h
 *
 * @author Xiaowen Jiang
 *
 * Header file for a Server class
 *
 */

#ifndef __SERVER_H
#define __SERVER_H

#include <iostream>
#include <string>
#include <unistd.h> 
#include <sys/types.h> 
#include <sys/socket.h> 
#include <arpa/inet.h> 
#include <netinet/in.h>

#include "ThreadPool.h"
#include "Request.h"
#include "Response.h"

using namespace std;

// definition server config sturct
struct Config{
  int port;
  string rootDocument;
};

class Server {
protected:
    /* variables needs to intial server socket */
    int sockfd;
    int opt;
    struct sockaddr_in servaddr;
    struct sockaddr_in cliaddr; 
    /* END: variables needs to intial server socket */
    struct Config config;
    void initialServer();
 
public:
    Server();
    Server(struct Config configuration);
    void start(); // start to listen and running the thread pool
    int getConnection(); // get client socket file descriptor
};

#endif