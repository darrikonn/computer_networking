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




Client request:

GET /hello.txt HTTP/1.1
User-Agent: curl/7.16.3 libcurl/7.16.3 OpenSSL/0.9.7l zlib/1.2.3
Host: www.example.com
Accept-Language: en, mi


Server response:

HTTP/1.1 200 OK
Date: Mon, 27 Jul 2009 12:28:53 GMT
Server: Apache
Last-Modified: Wed, 22 Jul 2009 19:15:56 GMT
ETag: "34aa387-d-1568eb00"
Accept-Ranges: bytes
Content-Length: 51
Vary: Accept-Encoding
Content-Type: text/plain

Hello World! My payload includes a trailing CRLF.


header: GET / HTTP/1.1
header: Host: localhost:2000
header: User-Agent: Mozilla/5.0 (X11; Linux x86_64; rv:49.0) Gecko/20100101 Firefox/49.0
header: Accept: text/html,application/xhtml+xml,application/xml;q=0.9,*/*;q=0.8
header: Accept-Language: en-US,en;q=0.5
header: Accept-Encoding: gzip, deflate
header: Cookie: color=red
header: Connection: keep-alive
header: Upgrade-Insecure-Requests: 1
header: Cache-Control: max-age=0
header: 
header:
