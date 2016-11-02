#ifndef CHATD_H_    // include guard
#define CHATD_H_

/*
 * Project includes
 */
#include <sys/select.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <glib.h>
#include "openssl/ssl.h"
#include "openssl/err.h"

/*
 * Macros
 */
#define CERTIFICATE "ssl/fd.crt"
#define PRIVATE_KEY "ssl/fd.key"
#define PASSWD "darri"
#define WELCOME "Welcome to the chat server!"

/*
 * Constant variables
 */
const size_t DATE_SIZE = 60;
const size_t LOG_MESSAGE_SIZE = 300;
const size_t MAX_CONNECTIONS = 256;
const size_t CONNECTION_TIME_OUT = 30;
const size_t MESSAGE_SIZE = 1024;

/*
 * Functions
 */
int sockaddr_in_cmp(const void*, const void*);
gint fd_cmp(gconstpointer,  gconstpointer, gpointer G_GNUC_UNUSED);
void getCurrentDate(char*);
void createInitialLog();
void logToFile(char*);
SSL_CTX* initializeSSL();
void checkConnectionTimeouts(time_t*, size_t, fd_set*);
void closeConnection(time_t*, int, fd_set*);

/*
 * Structs
 */
struct user {
  int fd;
  SSL* ssl;
  char* username;
  char* chatroom;
  char* ip;
  int port;
};

#endif  // CHATD_H_
