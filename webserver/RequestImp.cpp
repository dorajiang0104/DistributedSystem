/**
 * RequestImp.cpp
 *
 * @author Xiaowen Jiang
 *
 * Implementation file for the Request class
 *
 */

#import "Request.h"

using namespace std;

// function to parse the request 
// will not accept POST data at this time point.
void Request::parseRequest(string rawRequest) {
  istringstream resp(rawRequest);
  string line;
  int lineCount = 0;
  bool headerEnd = false;
  try {
    while (getline(resp, line) && (!headerEnd ? true : line != "\r")) {
      if (lineCount == 0) {
        // parse the first line
        size_t pos = 0;
        string delim = " ";
        string token;
        int j = 0;
        while ((pos = line.find(delim)) != string::npos) {
          token = line.substr(0, pos);
          if (j == 0) {
            this->method = token;
          } else if (j == 1) {
            this->uri = token;
          }
          line.erase(0, pos + delim.length());
          j++;
        }
        // remove /r/n after httpVersion string
        this->httpVersion = trim(line);

        // check if data we need is parsed correctly
        if (!startsWith(this->httpVersion, "HTTP/") || this->method == "" || this->uri == "") {
          this->badRequest = true;
          break;
        }
      } else {
        if (line != "\r") { // header end
          if (!headerEnd) {
            // parse header
            size_t pos = 0;
            string delim = ":";
            string key;
            string value;
            if ((pos = line.find(delim)) != string::npos) {
              key = line.substr(0, pos);
              key = trim(key);
              line.erase(0, pos + delim.length());
            } else {
              // if the header line not include a ':' consider it as a bad request
              this->badRequest = true;
              break;
            }
            value = trim(line);
            this->headers.insert(make_pair(key, value));
          } else {
            this->data = this->data.append(line);
          }
        } else {
          headerEnd = true;
        }
      }

      lineCount++;
    }
  } catch (...) {
    this->badRequest = true;
  }

  return;
}

Request::Request() {
  // default response
  this->headers = map<string, string>();
  this->method = "GET";
  this->httpVersion = "HTTP/1.0";
  this->uri = "/";
  this->data = "";
  this->badRequest = false;
}

Request::Request(string rawRequest) {
  this->data = "";
  this->badRequest = false;
  this->headers = map<string, string>();
  this->parseRequest(rawRequest);
}

/* getters */
string Request::getMethod() {
  return this->method;
}

string Request::getHttpVersion() {
  return this->httpVersion;
}

string Request::getURI() {
  return this->uri;
}

string Request::getHeader(string key) {
  return this->headers[key];
}

bool Request::isBadRequest() {
  return this->badRequest;
}
