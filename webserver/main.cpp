/**
 * main.cpp
 *
 * @author Xiaowen Jiang
 *
 */


#include "Helper.hpp"
#include "Server.h"

int main(int argc, char* argv[]) {
  // read args
  std::string rootDocument = getRootDocument(argc, argv);
  int port = getPort(argc, argv);

  // set configuration variables
  struct Config config;
  config.port = port;
  config.rootDocument = rootDocument;

  // start server
  Server server(config);
  server.start();

  return 0;
}