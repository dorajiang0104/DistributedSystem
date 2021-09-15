/**
 * Helper.hpp
 *
 * @author Xiaowen Jiang
 *
 * This file contains helper files which will be used in the project
 *
 */

#ifndef __HELPER_H
#define __HELPER_H

#include <sys/stat.h>
#include <algorithm> 
#include <iostream>
#include <string>
#include <fstream>

// trim from end of string (right)
inline std::string& rtrim(std::string& s)
{
  const char* t = " \t\n\r\f\v";
  s.erase(s.find_last_not_of(t) + 1);
  return s;
}

// trim from beginning of string (left)
inline std::string& ltrim(std::string& s)
{
  const char* t = " \t\n\r\f\v";
  s.erase(0, s.find_first_not_of(t));
  return s;
}

// trim from both ends of string (right then left)
inline std::string& trim(std::string& s)
{
  
  return rtrim(ltrim(s));
}

// determines whether a string begins with the characters of a specified string
inline bool startsWith(std::string const & value, std::string const & starting)
{
    if (starting.size() > value.size()) return false;
    return std::equal(starting.begin(), starting.end(), value.begin());
}

// determines whether a string ends with the characters of a specified string
inline bool endsWith(std::string const & value, std::string const & ending)
{
    if (ending.size() > value.size()) return false;
    return std::equal(ending.rbegin(), ending.rend(), value.rbegin());
}

// determins whether file exist
inline bool fileExist(const std::string& name) {
  struct stat stat_buf;   
  return stat(name.c_str(), &stat_buf) == 0; 
}

// determins whether file has read access
inline long isReadable(std::string filename) {
    struct stat stat_buf;
    int rc = stat(filename.c_str(), &stat_buf);
    return rc == 0 ? (stat_buf.st_mode & S_IRUSR) : false;
}

// use stat call to get file size
inline long getFileSize(std::string filename) {
    struct stat stat_buf;
    int rc = stat(filename.c_str(), &stat_buf);
    return rc == 0 ? stat_buf.st_size : -1;
}

// parse the -document_root argument
inline std::string getRootDocument(int argc, char* argv[]) {
  for(int i = 0; i < argc; i++) {
    std::string a = argv[i];
    if (a == "-document_root") {
      if (i + 1 >= argc) {
        std::cerr << "Error: missing document root value." << std::endl;
        exit(1);
      }
      std::string rootDocument = argv[i+1];
      if (fileExist(rootDocument)) {
        if (endsWith(rootDocument, "/")) {
          return rootDocument.substr(0, rootDocument.size()-1);
        } else {
          return rootDocument;
        }
        
      } else {
        std::cerr << "Error: document root is not exist." << std::endl;
        exit(1);
      }
    }
  }

  return "./";
}

// parse the -port argument
inline int getPort(int argc, char* argv[]) {
  for(int i = 0; i < argc; i++) {
    std::string a = argv[i];
    if (a == "-port") {
      if (i + 1 >= argc) {
        std::cerr << "Error: missing port value." << std::endl;
        exit(1);
      }
      
      try {
        int port = std::stoi(argv[i + 1]);
        return port;
      } catch (...) {
        std::cerr << "Error: port should be an integer." << std::endl;
        exit(1);
      }
    }
  }

  return 8000;
}

inline bool isJPG(std::string filename) {
  return (endsWith(filename, ".jpg") ||
    endsWith(filename, ".jpeg") ||
    endsWith(filename, ".JPG") ||
    endsWith(filename, ".JPEG")
  );
}

inline bool isICON(std::string filename) {
  return (endsWith(filename, ".ico") ||
    endsWith(filename, ".ICO")
  );
}

inline bool isGIF(std::string filename) {
  return (endsWith(filename, ".gif") ||
    endsWith(filename, ".GIF")
  );
}

inline bool isPNG(std::string filename) {
  return (endsWith(filename, ".png") ||
    endsWith(filename, ".png")
  );
}

inline bool isHTML(std::string filename) {
  return (endsWith(filename, ".html") ||
    endsWith(filename, ".HTML") ||
    endsWith(filename, ".htm") ||
    endsWith(filename, ".HTM") ||
    endsWith(filename, ".shtm") ||
    endsWith(filename, ".SHTML")
  );
}

inline bool isJavascript(std::string filename) {
  return endsWith(filename, ".js");
}

inline bool isStyle(std::string filename) {
  return endsWith(filename, ".css");
}

inline bool isWoff2(std::string filename) {
  return endsWith(filename, ".woff2");
}


#endif