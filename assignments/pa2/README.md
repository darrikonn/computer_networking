# HTTP server
This is a simple HTTP server programmed in C. The C programming language was chosen to practice secure programming and understand the security implications of server programming.

## Implementation
The server was implemented in the aforementioned programming language C.<br/>
The server accepts one command line argument: "port".

## Build
To build the application, run the following command from the source of the program:
> make -C ./src

## Run
To run the server, run the following command from the source of the program:
> ./src/httpd <port>

e.g. ./src/httpd 2000
