/* A UDP echo server with timeouts.
 *
 * Note that you will not need to use select and the timeout for a
 * tftp server. However, select is also useful if you want to receive
 * from multiple sockets at the same time. Read the documentation for
 * select on how to do this (Hint: Iterate with FD_ISSET()).
 */
#include "chat.h"

/* If someone kills the client, it should still clean up the readline
   library, otherwise the terminal is in a inconsistent state. The
   signal number is sent through a self pipe to notify the main loop
   of the received signal. This avoids a race condition in select. */
void signal_handler(int signum) {
  int _errno = errno;
  if (write(exitfd[1], &signum, sizeof(signum)) == -1 && errno != EAGAIN) {
    abort();
  }
  fsync(exitfd[1]);
  errno = _errno;
}

/*
 * Initialize exit file descriptor
 */
static void initialize_exitfd(void) {
  /* Establish the self pipe for signal handling. */
  if (pipe(exitfd) == -1) {
    perror("pipe()");
    exit(EXIT_FAILURE);
  }

  /* Make read and write ends of pipe nonblocking */
  int flags;        
  flags = fcntl(exitfd[0], F_GETFL);
  if (flags == -1) {
    perror("fcntl-F_GETFL");
    exit(EXIT_FAILURE);
  }        
  flags |= O_NONBLOCK;                /* Make read end nonblocking */
  if (fcntl(exitfd[0], F_SETFL, flags) == -1) {
    perror("fcntl-F_SETFL");
    exit(EXIT_FAILURE);
  }

  flags = fcntl(exitfd[1], F_GETFL);
  if (flags == -1) {
    perror("fcntl-F_SETFL");
    exit(EXIT_FAILURE);
  }
  flags |= O_NONBLOCK;                /* Make write end nonblocking */
  if (fcntl(exitfd[1], F_SETFL, flags) == -1) {
    perror("fcntl-F_SETFL");
    exit(EXIT_FAILURE);
  }

  /* Set the signal handler. */
  struct sigaction sa;
  sigemptyset(&sa.sa_mask);
  sa.sa_flags = SA_RESTART;           /* Restart interrupted reads()s */
  sa.sa_handler = signal_handler;
  if (sigaction(SIGINT, &sa, NULL) == -1) {
    perror("sigaction");
    exit(EXIT_FAILURE);
  }
  if (sigaction(SIGTERM, &sa, NULL) == -1) {
    perror("sigaction");
    exit(EXIT_FAILURE);
  }       
}

/*
 * Initialize the array with known values, i.e. 0
 */
void initializeArray(char* arr, int size) {
  memset(arr, '\0', size);
}

/*
 * Hash the password
 */
void hashPassword(unsigned char* hash, char* salt, char* passwd) {
  char hashString[HASH_STRING_SIZE];
  initializeArray(hashString, HASH_STRING_SIZE);
  strcpy(hashString, passwd);
  strcat(hashString, salt);

  for (int i = 0; i < HASH_ITERATIONS; i++) {
    SHA256_CTX context;
    SHA256_Init(&context);
    SHA256_Update(&context, (unsigned char*)hashString, strlen(hashString));
    SHA256_Final(hash, &context);
  }
}

/*
 * Authenticate the user
 */
void authenticate(char* new_user) {
  if (new_user == NULL || !g_strcmp0(new_user, "")) {
    printf("Invalid username\n");
    printLogIn();
    return;
  }

  char buffer[256]; // need to reuse the buffer
  /* Process and send this information to the server. */
  for (int i = 0; i < MAX_TRIES; i++) {
    char salt[SALT_SIZE], passwd[PASSWORD_SIZE];
    initializeArray(salt, SALT_SIZE);
    initializeArray(passwd, PASSWORD_SIZE);

    strcpy(buffer, "/user ");
    strncat(buffer, new_user, strlen(new_user));
    SSL_write(server_ssl, buffer, strlen(buffer));

    int n = SSL_read(server_ssl, salt, SALT_SIZE-1);
    salt[n] = '\0';

    getpasswd("Password: ", passwd, (int)PASSWORD_SIZE);

   
    unsigned char hash[SHA256_DIGEST_LENGTH];
    initializeArray(hash, SHA256_DIGEST_LENGTH);
    hashPassword(hash, salt, passwd);
    char* hash64 = g_base64_encode(hash, strlen((char*)hash));
    SSL_write(server_ssl, hash64, strlen(hash64));
    g_free(hash64);

    n = SSL_read(server_ssl, buffer, RESPONSE_SIZE);
    buffer[n] = '\0';

    if (!g_strcmp0(buffer, "1")) {
      printf("Welcome %s\n", new_user);
      user = new_user;

      free(prompt);
      prompt = strdup("> "); /* What should the new prompt look like? */
      rl_set_prompt(prompt);
      break;
    } else if (!g_strcmp0(buffer, "0")) {
      printf("Username or password incorrect!\n");
    }

    if (i == 2) {
      printf("Too many attempts!\n");
      printLogIn();
    }
  }
}

/*
 * Print log in message
 */
void printLogIn() {
  write(STDOUT_FILENO, "\nPlease, log in\nusername: ", 26);
  fsync(STDOUT_FILENO);
  rl_redisplay();
}

/* When a line is entered using the readline library, this function
   gets called to handle the entered line. Implement the code to
   handle the user requests in this function. The client handles the
   server messages in the loop in main(). */
void readline_callback(char* line) {
  char buffer[256];
  if (NULL == line) {
    rl_callback_handler_remove();
    signal_handler(SIGTERM);
    return;
  }
  if (strlen(line) > 0) {
    add_history(line);
  }
  if (user == NULL) {
    authenticate(line);
    return;
  }
  if ((strncmp("/bye", line, 4) == 0) ||
      (strncmp("/quit", line, 5) == 0)) {
    rl_callback_handler_remove();
    signal_handler(SIGTERM);
    return;
  }
  if (strncmp("/game", line, 5) == 0) {
    /* Skip whitespace */
    int i = 4;
    while (line[i] != '\0' && isspace(line[i])) { i++; }
    if (line[i] == '\0') {
      write(STDOUT_FILENO, "Usage: /game username\n",
          29);
      fsync(STDOUT_FILENO);
      rl_redisplay();
      return;
    }
    /* Start game */
    return;
  }
  if (strncmp("/join", line, 5) == 0) {
    int i = 5;
    /* Skip whitespace */
    while (line[i] != '\0' && isspace(line[i])) { i++; }
    if (line[i] == '\0') {
      write(STDOUT_FILENO, "Usage: /join chatroom\n", 22);
      fsync(STDOUT_FILENO);
      rl_redisplay();
      return;
    }
    char* chatroom = strdup(&(line[i]));

    /* Process and send this information to the server. */
    SSL_write(server_ssl, line, strlen(line));

    char res[RESPONSE_SIZE];
    int n = SSL_read(server_ssl, res, RESPONSE_SIZE);
    write(STDOUT_FILENO, res, n);

    /* Maybe update the prompt. */
    free(prompt);
    prompt = strdup("> "); /* What should the new prompt look like? */
    rl_set_prompt(prompt);
    return;
  }
  if (strncmp("/list", line, 5) == 0) {
    /* Query all available chat rooms */
    SSL_write(server_ssl, line, strlen(line));

    char res[RESPONSE_SIZE];
    int n = SSL_read(server_ssl, res, RESPONSE_SIZE);
    write(STDOUT_FILENO, res, n);

    return;
  }
  if (strncmp("/roll", line, 5) == 0) {
    /* roll dice and declare winner. */
    return;
  }
  if (strncmp("/say", line, 4) == 0) {
    /* Skip whitespace */
    int i = 4;
    while (line[i] != '\0' && isspace(line[i])) { i++; }
    if (line[i] == '\0') {
      write(STDOUT_FILENO, "Usage: /say username message\n",
          29);
      fsync(STDOUT_FILENO);
      rl_redisplay();
      return;
    }
    /* Skip whitespace */
    int j = i+1;
    while (line[j] != '\0' && isgraph(line[j])) { j++; }
    if (line[j] == '\0') {
      write(STDOUT_FILENO, "Usage: /say username message\n",
          29);
      fsync(STDOUT_FILENO);
      rl_redisplay();
      return;
    }
    char *receiver = strndup(&(line[i]), j - i - 1);
    char *message = strndup(&(line[j]), j - i - 1);

    /* Send private message to receiver. */

    return;
  }
  if (strncmp("/user", line, 5) == 0) {
    int i = 5;
    /* Skip whitespace */
    while (line[i] != '\0' && isspace(line[i])) { i++; }
    if (line[i] == '\0') {
      write(STDOUT_FILENO, "Usage: /user username\n", 22);
      fsync(STDOUT_FILENO);
      rl_redisplay();
      return;
    }
    char* new_user = strdup(&(line[i]));
    authenticate(new_user);

    /* Maybe update the prompt. */
    free(prompt);
    prompt = strdup("> "); /* What should the new prompt look like? */
    rl_set_prompt(prompt);
    return;
  }
  if (strncmp("/who", line, 4) == 0) {
    /* Query all available users */
    SSL_write(server_ssl, line, strlen(line));

    char res[RESPONSE_SIZE];
    int n = SSL_read(server_ssl, res, RESPONSE_SIZE);
    write(STDOUT_FILENO, res, n);

    return;
  }
  /* Sent the buffer to the server. */
  snprintf(buffer, 255, "Message: %s\n", line);
  write(STDOUT_FILENO, buffer, strlen(buffer));
  fsync(STDOUT_FILENO);
}

int main(int argc, char **argv) {
  // validate and parse parameters
  if (argc != 3) {
    fprintf(stderr, "Usage: %s <hostname> <port>\n", argv[0]);
    exit(EXIT_FAILURE);
  }

  // get the hostname
  char* hostname = argv[1];

  // string to long(string, endptr, base)
  char* p;
  long port = strtol(argv[2], &p, 10);
  if (errno != 0 || *p != '\0' || port > INT_MAX || port < INT_MIN) {
    fprintf(stderr, "Port number is invalid!\n");
    exit(EXIT_FAILURE);
  }

  initialize_exitfd();

  // initialize OpenSSL
  SSL_library_init();
  SSL_load_error_strings();
  SSL_CTX *ssl_ctx = SSL_CTX_new(TLSv1_client_method());
  if (ssl_ctx == NULL) {
    perror("SSL_CTX_new()");
    exit(EXIT_FAILURE);
  }

  // get host
  struct hostent* host = gethostbyname(hostname);
  if (host == NULL) {
    perror("gethostbyname()");
    exit(EXIT_FAILURE);
  }

  // get available socket
  server_fd = socket(PF_INET, SOCK_STREAM, 0);

  // network functions need arguments in network byte order instead of host byte order.
  // the macros htonl and htons convert the value
  struct sockaddr_in server;
  memset(&server, 0, sizeof(server));
  server.sin_family = AF_INET;
  server.sin_addr.s_addr = *(long*)(host->h_addr);
  server.sin_port = htons(port);

  // connect to the server
  if (connect(server_fd, (struct sockaddr*)& server, sizeof(server)) != 0) {
    close(server_fd);
    perror("connect()");
    exit(EXIT_FAILURE);
  }

  // create ssl
  server_ssl = SSL_new(ssl_ctx);

  // use the socket for the SSL connections
  //SSL_set_fd(server_fd, server_fd);

  // use BIOs them instead of the socket.
  BIO* sbio = BIO_new_socket(server_fd, BIO_NOCLOSE);
  if (sbio == NULL) {
    perror("BIO_new_socket()");
    exit(EXIT_FAILURE);
  }
  SSL_set_bio(server_ssl, sbio, sbio);

  // set up secure connection to the chatd server.
  if (SSL_connect(server_ssl) == -1) {
    perror("SSL_connect()");
    exit(EXIT_FAILURE);
  }

  // read characters from the keyboard while waiting for input.
  //prompt = strdup("> ");
  rl_callback_handler_install(prompt, (rl_vcpfunc_t*) &readline_callback);
  for (;;) {
    fd_set rfds;
    struct timeval timeout;

    /* You must change this. Keep exitfd[0] in the read set to
       receive the message from the signal handler. Otherwise,
       the chat client can break in terrible ways. */
    FD_ZERO(&rfds);
    FD_SET(STDIN_FILENO, &rfds);
    FD_SET(exitfd[0], &rfds);
    FD_SET(server_fd, &rfds);
    timeout.tv_sec = 3;
    timeout.tv_usec = 0;

    int maxfd = server_fd > STDIN_FILENO ? server_fd : STDIN_FILENO;
    if (exitfd[0] > maxfd) {
      maxfd = exitfd[0];
    }
    int r = select(maxfd + 1, &rfds, NULL, NULL, &timeout);
    if (r < 0) {
      if (errno == EINTR) {
        /* This should either retry the call or
           exit the loop, depending on whether we
           received a SIGTERM. */
        continue;
      }
      /* Not interrupted, maybe nothing we can do? */
      perror("select()");
      break;
    }
    if (r == 0) {
      /*
      write(STDOUT_FILENO, "No message?\n", 12);
      fsync(STDOUT_FILENO);
      */
      /* Whenever you print out a message, call this
         to reprint the current input line. */
      /*rl_redisplay();*/
      continue;
    }
    if (FD_ISSET(exitfd[0], &rfds)) {
      /* We received a signal. */
      int signum;
      for (;;) {
        if (read(exitfd[0], &signum, sizeof(signum)) == -1) {
          if (errno == EAGAIN) {
            break;
          } else {
            perror("read()");
            exit(EXIT_FAILURE);
          }
        }
      }
      if (signum == SIGINT) {
        /* Don't do anything. */
      } else if (signum == SIGTERM) {
        /* Clean-up and exit. */
        break;
      }
    }
    if (FD_ISSET(STDIN_FILENO, &rfds)) {
      rl_callback_read_char();
    }
    if (FD_ISSET(server_fd, &rfds)) {
      /* Handle messages from the server here! */
      char message[MESSAGE_SIZE];
      int n = SSL_read(server_ssl, message, sizeof(message)-1);
      message[n] = '\0';
      printf("\"%s\"\n", message);
      printLogIn();
    }
  }

  // close connections
  SSL_free(server_ssl);
  close(server_fd);
  SSL_CTX_free(ssl_ctx);

  return 0;
}
