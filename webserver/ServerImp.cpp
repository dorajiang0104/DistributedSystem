/**
 * ServerImp.cpp
 *
 * @author Xiaowen Jiang
 *
 * Implementation file for the Server class
 *
 */

#import "Server.h"

using namespace std;

#define BUFFER_LENGHT 2048

/**
 * class ConnectionTask
 *
 * Task class which will executed by thread workers
 * It read the request and send back the response
 * after response sent, the connection to the client will be closed.
 *
 */

class ConnectionTask: public Task {
  protected:
    // static int count;
    int sockfd;
    string rootDocument;
  public:
    ConnectionTask(int fd, string rootDir) {
      this->sockfd = fd;
      this->rootDocument = rootDir;
    }
    int Run() {
      char* buffer = (char*) malloc(BUFFER_LENGHT);
      memset(buffer, 0, BUFFER_LENGHT);
      read(this->sockfd, buffer, 2047);
      Request req(buffer);
      cout << "Request: " << req.getMethod() << " \"" << req.getURI() << "\" from " << req.getHeader("User-Agent") << endl;
      Response res(this->sockfd, req, this->rootDocument);
      res.send();
      close(this->sockfd);
      return 0;
    }
};

// set the server options and bind the server fd to the specific port
void Server::initialServer() {
  this->opt = 1;
  // Creating socket file descriptor 
  if ((this->sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) { 
      cout << "socket creation failed" << endl; 
      exit(EXIT_FAILURE); 
  }

  memset(&this->servaddr, 0, sizeof(this->servaddr));
  
  // Filling server information
  this->servaddr.sin_family = AF_INET; // IPv4
  this->servaddr.sin_addr.s_addr = INADDR_ANY;
  this->servaddr.sin_port = htons(this->config.port);

  setsockopt(this->sockfd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &this->opt, sizeof(this->opt));

  // Bind the socket with the server address 
  if (bind(this->sockfd, (const struct sockaddr *)&servaddr, sizeof(servaddr)) < 0 ) { 
    cout << "bind failed" << endl;
    close(this->sockfd);
    exit(EXIT_FAILURE); 
  }

  return;
}

Server::Server() {
  this->config.port = 8000;
  this->config.rootDocument = "./web";
  initialServer();
}

Server::Server(struct Config configuration) {
  this->config.port = configuration.port;
  this->config.rootDocument = configuration.rootDocument;
  initialServer();
}

// start the threadpool and listening
void Server::start() {
  // start thread pool
  ThreadPool Pool(30);

  // listen on the server fd
  if (listen(this->sockfd, 10) == -1) {
    cout << "fail listen on port: " << this->config.port << endl; 
    close(this->sockfd);
    exit(EXIT_FAILURE); 
  }

  std::cout << "Server Listening on port: " << this->config.port << std::endl;

  while(1) {
    // when received a new connection, add a new task.
    int connectfd = this->getConnection();
    if (connectfd > 0) {
      // add task
      Task* ta = new ConnectionTask(connectfd, this->config.rootDocument);
      Pool.AddTask(ta);
    } else {
      std::cout << "Connection failed." << std::endl;
    }
  }
}

// return the client socket descriptor
int Server::getConnection() {
  int addrlen = sizeof(this->cliaddr);
  int clientfd = accept(this->sockfd, (struct sockaddr *)&this->cliaddr, (socklen_t*)&addrlen);
  return clientfd;
}