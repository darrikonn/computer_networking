# TFTP server
This is a TFTP (Trivial File Transfer Protocol) server programmed in the C programming language.<br/>
The server was conformed to the TFTP protocol defined in RFC1350.

## Implementation
The server was implemented in the aforementioned programming language C. The server only allows the client to download files, but not uploading.<br/>
The server accepts two command line arguments: "port" and "directory".<br/>
The server only sends files that are in the given directory; for security reasons.

## Build
To build the application, run the following command from the source of the program:
> make -C ./src

## Run
To run the server, run the following command from the source of the program:
> ./src/tftp <port> <directory>

e.g. ./src/tftp 2000 data

## Download a file from a client
To download a file from the client tftp, make sure you have tftp client installed and run the following command:
> tftp <ip_address> <port> -c get <requested_file>

The following commands fetch the files from the data directory that is located on the server side:
> tftp 127.0.0.1 2000 -c get example_data1<br/>
  tftp 127.0.0.1 2000 -c get example_data2<br/>
  tftp 127.0.0.1 2000 -m binary -c get example_data3

## Packets
##### RRQ/WRQ packet
 2 bytes     char\*   byte  char\*  byte<br/>
+--------+----------+------+------+-----+<br/>
| OpCode | Filename |   0  | Mode |  0  |<br/>
+--------+----------+------+------+-----+<br/>

##### DATA packet
 2 bytes   2 bytes  n bytes<br/>
+--------+---------+--------+<br/>
| OpCode | Block # |  Data  |<br/>
+--------+---------+--------+<br/>

##### ACK packet
 2 bytes   2 bytes<br/>
+--------+---------+<br/>
| OpCode | Block # |<br/>
+--------+---------+<br/>

##### Data packet
 2 bytes   2 bytes  n bytes<br/>
+--------+---------+--------+<br/>
| OpCode | Block # |  Data  |<br/>
+--------+---------+--------+<br/>

##### ERROR packet
 2 bytes   2 bytes   char\*  byte<br/>
+--------+---------+--------+-----+<br/>
| OpCode | ErrCode | ErrMsg |  0  |<br/>
+--------+---------+--------+-----+<br/>
