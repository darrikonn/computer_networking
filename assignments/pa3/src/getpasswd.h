#ifndef GETPASSWD_H_    // include guard
#define GETPASSWD_H_

/*
 * Project includes
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <termios.h>
#include <unistd.h>

/*
 * Functions
 */
void getpasswd(const char*, char*, size_t);

#endif  // GETPASSWD_H_
