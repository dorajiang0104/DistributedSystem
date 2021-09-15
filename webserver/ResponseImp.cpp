/**
 * ResponseImp.cpp
 *
 * @author Xiaowen Jiang
 *
 * Implementation file for the Response class
 *
 */

#include "Response.h"

#define CHUNK_SIZE 4096

// static data
const string statusOK = "200 OK";
const string statusMovedPermanetly = "301 Moved Permanently";
const string statusBadRequest = "400 Bad Request";
const string statusForbidden = "403 Forbidden";
const string statusNotFound = "404 Not Found";
const string statusInternalError = "500 Internal Server Error";
const string statusNotImplement = "501 Not Implemented";

const string bodyStart =
"<!DOCTYPE html>\r\n"
"<html><head><title>Hello!</title></head><body><h1>";

const string bodyEnd =
"</h1></body></html>\r\n";


Response::Response(int fd, Request req, string rootDir) {
  this->badRequest = req.isBadRequest();
  this->sockfd = fd;
  this->status = 500;
  this->httpVersion = req.getHttpVersion();
  string filename = req.getURI();
  if (req.getURI() == "/") {
    filename = filename.append("index.html");
  }
  this->uri = rootDir.append(filename);
  this->method = req.getMethod();

  // always generate Date header
  time_t rawtime;
  struct tm * timeinfo;

  time ( &rawtime );
  timeinfo = localtime(&rawtime);
  char rawDate[30];
  strftime(rawDate, sizeof rawDate, "%a, %d %b %Y %H:%M:%S GMT", timeinfo);
  string date(rawDate);
  this->headers.insert(make_pair("Date", date));
}

string Response::getStatusText() {
  switch(this->status) {
    case 200:
      return statusOK;
    case 301:
      return statusMovedPermanetly;
    case 400:
      return statusBadRequest;
    case 403:
      return statusForbidden;
    case 404:
      return statusNotFound;
    case 501:
      return statusNotImplement;
    default:
      return statusInternalError;
  }
}

// generate http header
string Response::generateHeader() {
  string headerString;
  // headerString = headerString.append(this->httpVersion);
  // right now, we just support HTTP/1.0
  headerString = headerString.append("HTTP/1.0");
  headerString = headerString.append(" ");
  headerString = headerString.append(this->getStatusText());
  headerString = headerString.append("\r\n");
  // ad headers
  map<string, string>::iterator it;
  for (it = this->headers.begin(); it != this->headers.end(); it++) {
    headerString = headerString.append(it->first);
    headerString = headerString.append(": ");
    headerString = headerString.append(it->second);
    headerString = headerString.append("\r\n");
  }

  headerString = headerString.append("\r\n");
  return headerString;
}

// generate a simple html page for error status code
string Response::generateBody() {
  string bodyString = "";
  bodyString = bodyString.append(bodyStart);
  bodyString = bodyString.append(this->getStatusText());
  bodyString = bodyString.append(bodyEnd);

  return bodyString;
}

// setter for headers
void Response::setHeader(string key, string value) {
  if (this->headers.find("f") == this->headers.end()) {
    this->headers.insert(make_pair(key, value));
  } else {
    this->headers[key] = value;
  }
}

// getter for headers
string Response::getHeader(string key) {
  return this->headers[key];
}

// generate response & send
int Response::send() {
  /* route */
  
  if (this->badRequest) {
    // if received a bad request, skip everything and send
    this->status = 400;
  } else {
    // we only support GET method at this project.
    if (this->method != "GET") {
      this->status = 501;
    } else {
      errno = 0;
      if (fileExist(this->uri)) {
        if (isReadable(this->uri)) {
          long fileSize = getFileSize(this->uri);
          if (fileSize) {
            if (isJPG(this->uri)) {
              this->setHeader("Content-Type", "image/jpeg");
            } else if (isPNG(this->uri)) {
              this->setHeader("Content-Type", "image/png");
            } else if (isGIF(this->uri)) {
              this->setHeader("Content-Type", "image/gif");
            } else if (isICON(this->uri)) {
              this->setHeader("Content-Type", "image/x-icon");
            } else if (isHTML(this->uri)){
              this->setHeader("Content-Type", "text/html");
            } else if (isStyle(this->uri)) {
              this->setHeader("Content-Type", "text/css");
            } else if (isWoff2(this->uri)) {
              this->setHeader("Content-Type", "font/woff2");
            } else if (isJavascript(this->uri)) {
              this->setHeader("Content-Type", "text/javascript");
            } else {
              this->setHeader("Content-Type", "text/plain");
            }
            this->setHeader("Content-Length", to_string(fileSize));
            this->status = 200;
          } else {
            this->status = 404;
          }
        } else {
          this->status = 403;
        }
      } else {
          this->status = 404;
      }
    }
  }

  string headerString = this->generateHeader();
  
  ::send(this->sockfd, headerString.c_str(), headerString.size(), 0);

  if (this->status >= 400) {
    string bodyString = this->generateBody();
    ::send(this->sockfd, bodyString.c_str(), bodyString.size(), 0);
    return 1;
  }

  // read data from files, the file has to existed since we checked above.
  if (this->status == 200) {
    char* buffer = (char*) malloc(CHUNK_SIZE);
    memset(buffer, 0, CHUNK_SIZE);
    ifstream ifile(this->uri);
    while (ifile) {
      ifile.read(buffer, CHUNK_SIZE);
      size_t count = ifile.gcount();
      if (!count) {
        break;
      }
      
      // keep fetch and send until it reach to the EOF
      ::send(this->sockfd, buffer, CHUNK_SIZE, 0);  
    }
    ifile.close();
    cout << "Response with Status " << this->status << endl; 
    delete[] buffer;
    return 0;
  }
  return 0;
}