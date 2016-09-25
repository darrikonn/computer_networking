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
#include <string.h>         // memset
#include <unistd.h>         // close
#include <glib.h>

/*
 * Constant variables
 */
const int MESSAGE_SIZE = 512;
const int TYPE_OF_REQUEST_SIZE = 5;

/*
 * Functions
 */
void getTypeOfRequestSize(char* toR, char* req);
void initializeArray(char* arr, int size);

#endif // HTTPD_H_
