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
const int MESSAGE_SIZE = 512; // kringum 1500, max ip size minus header
const int HTML_SIZE = 512;

/*
 * Functions
 */
void constructHashTable(GHashTable* hash, char* msg);
void generateHTML(GHashTable* hash, char* html);
void initializeArray(char* arr, int size);

#endif // HTTPD_H_
