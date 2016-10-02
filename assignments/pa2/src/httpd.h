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
#include <time.h>
#include <sys/types.h>
#include <sys/select.h>

/*
 * Constant variables
 */
const size_t MESSAGE_SIZE = 2048;
const size_t HTML_SIZE = 1500;
const size_t DATE_SIZE = 60;
const size_t DATA_SIZE = 1024;
const size_t LOG_MESSAGE_SIZE = 300;
const size_t MAX_CONNECTIONS = 256;
const size_t CONNECTION_TIME_OUT = 30;
const size_t BODY_SIZE = 60;
const size_t QUERY_DATA_SIZE = 100;

/*
 * Macros
 */
#define HTTP_OK 200
#define HTTP_CREATED 201
#define HTTP_BAD_REQUEST 400
#define HTTP_FORBIDDEN 403
#define HTTP_NOT_FOUND 404
#define HTTP_INTERNAL_SERVER_ERROR 500

/*
 * Enums
 */
typedef enum {
    GET, POST, HEAD, ERROR
} RequestType;

/*
 * Functions
 */
void constructHashTable(GHashTable* hash, char* message);
int getPostData(char* postData, GHashTable* hash, RequestType type);
int generateHTML(GHashTable* hash, char* html, char* date, char* addr, int port, RequestType type);
int generateErrorHTML(char* html, char* errorMessage);
void generateServerResponse(GHashTable* hash, char* message, char* date, char* html, 
        int contentLength, RequestType type);
void getCurrentDate(char* date);
void getClientAddr(struct sockaddr_in client, char* addr);
void initializeArray(char* arr, int size);
RequestType getRequestType(char* type);
void createInitialLog();
void createRequestLog(char* date, char* addr, int port, GHashTable* hash, int resp);
void logToFile(char* message);
void checkConnectionTimeouts(time_t* clientsTimeOuts, size_t ctoSize, fd_set* fdset);
void closeConnection(time_t* clientsTimeOuts, int connfd, fd_set* fdset);

#endif // HTTPD_H_
