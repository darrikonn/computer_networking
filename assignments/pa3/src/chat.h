#ifndef CHAT_H_     // include guard
#define CHAT_H_

/*
 * Project includes
 */
#include <assert.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <ctype.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <termios.h>
#include <signal.h>
#include <openssl/ssl.h> // Secure socket layer headers
#include <openssl/err.h>
#include <readline/readline.h> // For nicer interaction, we use the GNU readline library.
#include <readline/history.h>
#include <netdb.h>  // used for gethostbyname

/*
 * Constant variables
 */
const size_t MESSAGE_SIZE = 1024;
const size_t RESPONSE_SIZE = 1024;

/*
 * Static variables
 */
// This variable holds a file descriptor of a pipe on which we send a
// number if a signal is received.
static int exitfd[2];
// The next two variables are used to access the encrypted stream to
// the server. The socket file descriptor server_fd is provided for
// select (if needed), while the encrypted communication should use
// server_ssl and the SSL API of OpenSSL.
static int server_fd;
static SSL* server_ssl;
// This variable shall point to the name of the user. The initial value
// is NULL. Set this variable to the username once the user managed to be 
// authenticated.
static char* user;
// This variable shall point to the name of the chatroom. The initial
// value is NULL (not member of a chat room). Set this variable whenever
// the user changed the chat room successfully.
static char* chatroom;
// This prompt is used by the readline library to ask the user for
// input. It is good style to indicate the name of the user and the
// chat room he is in as part of the prompt.
static char* prompt;

/*
 * Functions
 */
void signal_handler(int);
static void initialize_exitfd();
void readline_callback(char*);

#endif  // CHAT_H_
