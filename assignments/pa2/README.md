# HTTP server
This is a simple HTTP server; programmed in C. The C programming language was chosen to practice secure programming and understand the security implications of server programming.<br/>
The source of the program is one level above /src. The reason for that is because, the server will log its requests to a log file.

## Implementation
The server was implemented in the aforementioned programming language C.<br/>
The server allows the following requests: GET, POST and HEAD. There can be up to 256 parallel connections, where the connection can be persistent if the client requests it. <br/>
The server can also accept queries to set the background color of the HTML page.</br>
The server uses cookies to "remember" the last requested background color of the HTML page.<br/>
The server accepts one command line argument: "port".<br/>
**NOTE**: I chose the name "color" instead of "colour" for the color page. I thought it was more appropriate.

## Build
To build the application, run the following command from the source of the program:
> make -C ./src

## Run
To run the server, run the following command from the source of the program:
> ./src/httpd [port]

e.g. ./src/httpd 2000

## Request from server
To make a request to the server, visit the following index page in a web browser:
> localhost:[port]

Other possible urls are:
> localhost:[port]/color?bg=red<br/>
  localhost:[port]/color<br/>
  localhost:[port]/test?key=value

<br/>

You can also perform POST and HEAD requests any way you like, e.g. with curl:
> curl -X POST -d "some data" localhost:[port]<br/>
  curl -I localhost:[port] 
