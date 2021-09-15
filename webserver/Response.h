/**
 * Response.h
 *
 * @author Xiaowen Jiang
 *
 * Header file for Reponse class
 * This class will generate the response according to the request.
 * Also, it provides a function to send the response back to server.
 *
 */

#ifndef __RESPONSE_H
#define __RESPONSE_H

#include <stdlib.h>
#include <time.h>
#include <iostream>
#include <fstream>
#include <string>
#include <map>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include "Request.h"
#include "Helper.hpp"

using namespace std;  

class Response {
  protected:
    bool badRequest; // if need to response as 404
    int sockfd;
    int status; // status code
    string uri;
    string httpVersion;
    string method;
    map<string, string> headers;
    string getStatusText();
    string generateHeader();
    string generateBody();
  public:
    // Response(int fd);
    Response(int fd, Request req, string rootDir);
    void setHeader(string key, string value);
    string getHeader(string key);
    int send(); // generate & send response back to server
};

#endif