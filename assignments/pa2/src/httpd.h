#ifndef HTTPD_H_    // include guard
#define HTTPD_H_

/*
 * Project includes
 */
#include <stdio.h>
#include <limits.h>
#include <errno.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>      // inet_ntop
#include <string.h>         // memset
#include <unistd.h>         // close
#include <glib.h>

/*
 * Constant variables
 */
const int MESSAGE_SIZE = 2048; // kringum 1500, max ip size minus header
const int HTML_SIZE = 1500;
const int DATE_SIZE = 60;
const int DATA_SIZE = 1024;

/*
 *
 */
typedef enum {
    GET, POST, HEAD, ERROR
} RequestType;

/*
 * Functions
 */
void constructHashTable(GHashTable* hash, char* message);
int generateHTML(GHashTable* hash, char* html, char* addr, int port, char* postData);
void generateServerResponse(char* message, char* html, int contentLength, RequestType type);
void getCurrentDate(char* date);
void getClientAddr(struct sockaddr_in client, char* addr);
void initializeArray(char* arr, int size);

#endif // HTTPD_H_
