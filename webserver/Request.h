/**
 * Request.h
 *
 * @author Xiaowen Jiang
 *
 * Header file for a ReadOnly Request class
 * This class will parse the raw request.
 *
 */

#ifndef __REQUEST_H
#define __REQUEST_H

#include <stdlib.h>
#include <iostream>
#include <sstream>
#include <string>
#include <map>
#include "Helper.hpp"

using namespace std;  

class Request {
protected:
  bool badRequest; // if request is a bad request, if true a 404 response will generate
  string method;
  string httpVersion;
  string uri;
  map<string, string> headers;
  void parseRequest(string rawRequest); // function to parse request data
  string data; // @NotImplemented: request data, for POST method
 
public:
  Request();
  Request(string rawRequest);
  string getMethod();
  string getHttpVersion();
  string getURI();
  string getHeader(string key);
  bool isBadRequest();
};

#endif