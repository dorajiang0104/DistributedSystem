COEN 317: Distributed Systems Project Assignment 1
By Xiaowen Jiang
Apr/26/2021


For this assignment, I use C++ to build a functional web server . To increase the 
performance of the server, I also implemented a simple thread pool to handle scenario
that server needs to handle multiple HTTP requests. At this point, the server
is supporting HTTP/1.0 protocol and GET method only. And server is able to handle the
200, 400, 402, 403 status code. and if server received any request with the method is
not GET, we will response with status code 501 - method not implemented.

The submitted file includes:

/web                // web content use for test
/main.cpp           // main function
/Helper.hpp         // definition of helper functions
/Server.h           // header file for Server class, also a ConnectedTask is defined
                    // here
/ServerImp.cpp      // implementation file for Server class

/Task.h             // header file for Task class,
                    // It contains an virtual method Run(),
                    // Instances of this class can be added into Thread Pool and
                    // executed by threads.
/ThreadPool.h       // header file for ThreadPool class
/ThreadPoolImp.cpp  // implementation file for ThreadPool class

/Request.h          // header file for Request class
/Request.cpp        // implementation file for Request class   
/Response.h         // header file for Response class
/ResponeImp.cpp     // implementation file for Response class

/Makefile           // make file

Build & Run:
To make sure the build is clean, please run `make clean` under the folder first.

Type:

  `$ make`

in the program folder, an executable `server` file will be compiled.
To run the program:

Type:

  `$ ./server -document_root <rootDocument> -port <portNumber>`

Please make sure the directory you passed into your the document_root argument
is existed. 

After running the server, you might get an error message said `bind failed`, that
might happen because the port you are trying to bind is still in use. you can 
manually kill the process using that port/change another port. If the reason is 
previous running  of the server instance, you can also wait a while and start the 
server again.

Once you started the server, you can use any browser to hit the server with
'http://localhost:<portNumber>'.

