/* A TCP echo server with timeouts.
 *
 * Note that you will not need to use select and the timeout for a
 * tftp server. However, select is also useful if you want to receive
 * from multiple sockets at the same time. Read the documentation for
 * select on how to do this (Hint: Iterate with FD_ISSET()).
 */
#include "chatd.h"

/* 
 * This can be used to build instances of GTree that index on the address of a connection. 
 */
int sockaddr_in_cmp(const void *addr1, const void *addr2) {
  const struct sockaddr_in *_addr1 = addr1;
  const struct sockaddr_in *_addr2 = addr2;

  /* If either of the pointers is NULL or the addresses
     belong to different families, we abort. */
  g_assert((_addr1 == NULL) || (_addr2 == NULL) ||
      (_addr1->sin_family != _addr2->sin_family));

  if (_addr1->sin_addr.s_addr < _addr2->sin_addr.s_addr) {
    return -1;
  } else if (_addr1->sin_addr.s_addr > _addr2->sin_addr.s_addr) {
    return 1;
  } else if (_addr1->sin_port < _addr2->sin_port) {
    return -1;
  } else if (_addr1->sin_port > _addr2->sin_port) {
    return 1;
  }
  return 0;
}

/* 
 * This can be used to build instances of GTree that index on the file descriptor of a connection. 
 */
gint fd_cmp(gconstpointer fd1,  gconstpointer fd2, gpointer G_GNUC_UNUSED data) {
  return GPOINTER_TO_INT(fd1) - GPOINTER_TO_INT(fd2);
}

/*
 * Get the current date on ISO 8601 form
 */
void getCurrentDate(char* date) {
  time_t rawtime;
  struct tm* timeinfo;
  time(&rawtime);
  timeinfo = localtime(&rawtime);

  strftime(date, DATE_SIZE, "%a, %d %b %G %T GMT", timeinfo);
}

/*
 * Create a log message
 */
void createRequestLog(char* addr, int port, char* msg) {
  char date[DATE_SIZE];
  getCurrentDate(date);
  char message[LOG_MESSAGE_SIZE];
  g_snprintf(message, LOG_MESSAGE_SIZE, "%s : %s:%d %s",
      date, addr, port, msg);

  logToFile(message);
}

/*
 * Create the initial log message
 */
void createInitialLog() {
  char date[DATE_SIZE];
  getCurrentDate(date);
  char message[LOG_MESSAGE_SIZE];
  g_snprintf(message, LOG_MESSAGE_SIZE, "%s\n%s -> Starting to log:\n%s", 
      "=================================================",
      date, 
      "=================================================");

  logToFile(message);
}

/*
 * Log the message to a log file 
 */
void logToFile(char* message) {
  // open file in appending mode
  FILE* f = fopen("log/chatd.log", "a");
  if (f == NULL) {
    perror("fopen()");
    exit(EXIT_FAILURE);
  }
  fprintf(f, "%s\n", message);
  fclose(f);
}

/*
 * Get the address of the client
 */
void getClientAddr(struct sockaddr_in client, char* addr) {
    // get the ip address of the client on human readable form
    inet_ntop(AF_INET, &(client.sin_addr), addr, INET_ADDRSTRLEN);
}

/*
 * Initialize the SSL
 */
SSL_CTX* initializeSSL() {
  SSL_library_init();
  // add cryptos and initialize OpenSSL
  OpenSSL_add_all_algorithms();
  SSL_load_error_strings();
  SSL_CTX *ssl_ctx = SSL_CTX_new(TLSv1_server_method());
  if (ssl_ctx == NULL) {
    perror("SSL_CTX_new()");
    exit(EXIT_FAILURE);
  }

  // get certificate
  if (SSL_CTX_use_certificate_file(ssl_ctx, CERTIFICATE, SSL_FILETYPE_PEM) <= 0) {
    perror("SSL_CTX_use_certificate_file()");
    exit(EXIT_FAILURE);
  }

  // load password
  SSL_CTX_set_default_passwd_cb_userdata(ssl_ctx, PASSWD);

  // get private key file
  if (SSL_CTX_use_PrivateKey_file(ssl_ctx, PRIVATE_KEY, SSL_FILETYPE_PEM) <= 0) {
    perror("SSL_CTX_use_PrivateKey_file()");
    exit(EXIT_FAILURE);
  }

  // verify private key
  if (!SSL_CTX_check_private_key(ssl_ctx)) {
    perror("public certificate and private key are not matching");
    exit(EXIT_FAILURE);
  }

  return ssl_ctx;
}

/*
 * Get the names of all the users
 */
void getAllUserNamesOfTree(struct sockaddr_in* client, struct user_s* user, char* users) {
  (void)client;
  g_snprintf(users+strlen(users), RESPONSE_SIZE, "%s\n", user->username);
}

/*
 * Initialize the array with known values, i.e. 0
 */
void initializeArray(char* arr, int size) {
  memset(arr, '\0', size);
}

/*
 * Get the names of all the chatrooms
 */
void getAllNamesOfChatRooms(char* name, gpointer tmp, char* rooms) {
  (void)tmp;
  printf("name: %s\n", name);
  g_snprintf(rooms+strlen(rooms), RESPONSE_SIZE, "%s\n", name);
}

/*
 * Join a room
 */
int joinRoom(struct user_s* user, struct sockaddr_in* client, char* room, char* res) {
  // check if it is the users room
  if (g_strcmp0(user->chatroom, room)) {
    short created = 0;

    // remove user from current room
    struct chatroom_s* chatroom = g_tree_lookup(chatroom_t, user->chatroom);
    if (chatroom != NULL) {
      chatroom->list = g_list_remove(chatroom->list, client);
    }

    // add the user to the new room, create if it doesn't exist
    chatroom = g_tree_lookup(chatroom_t, room);
    if (chatroom == NULL) {
      // create the new room
      chatroom = g_new0(struct chatroom_s, 1);
      chatroom->name = g_strdup(room);

      // insert the chatroom to the chatroom tree
      g_tree_insert(chatroom_t, room, chatroom);

      created = 1;
    }

    // add the user to the chatroom list
    chatroom->list = g_list_append(chatroom->list, client);

    // update the users chatroom
    user->chatroom = chatroom->name;

    return g_snprintf(res, RESPONSE_SIZE, "Joined%s chatroom: %s\n", 
        created ? " and created" : "", 
        room);
  }

  return g_snprintf(res, RESPONSE_SIZE, "Already a member of chatroom: %s\n", room);
}

/*
 * Handle the client requests
 */
void handleRequests(struct user_s* user, struct sockaddr_in* client, char* message) {
  char res[RESPONSE_SIZE];
  initializeArray(res, RESPONSE_SIZE);
  if (g_str_has_prefix(message, "/who")) {
    if (!g_tree_height(user_t)) {
      g_stpcpy(res, "No users");
    } else {
      g_tree_foreach(user_t, (GTraverseFunc)getAllUserNamesOfTree, res);
    }

    SSL_write(user->ssl, res, sizeof(res));
  } else if (g_str_has_prefix(message, "/list")) {
    printf("height: %d\n", g_tree_height(chatroom_t));
    if (!g_tree_height(chatroom_t)) {
      g_stpcpy(res, "No rooms");
    } else {
      g_tree_foreach(chatroom_t, (GTraverseFunc)getAllNamesOfChatRooms, res);
    }

    SSL_write(user->ssl, res, sizeof(res));
  } else if (g_str_has_prefix(message, "/join")) {
    gchar** tmp = g_strsplit(message, " ", 2);

    int n = joinRoom(user, client, tmp[1], res);
    SSL_write(user->ssl, res, n);
    g_strfreev(tmp);
  }
}

/*
 * Traverse the user tree
 */
void traverseTree(struct sockaddr_in* client, struct user_s* user, fd_set* tempset) {
  (void)client;
  if (FD_ISSET(user->fd, tempset)) {
    char message[MESSAGE_SIZE];
    int n = SSL_read(user->ssl, message, MESSAGE_SIZE-1);
    message[n] = '\0';
    printf("%s\n", message);
    handleRequests(user, client, message);
  }
}

int main(int argc, char **argv) {
  struct sockaddr_in server, client;

  if (argc != 2) {
    fprintf(stderr, "Usage: %s <port>\n", argv[0]);
    exit(EXIT_FAILURE);
  }

  // string to long(string, endptr, base)
  char* p;
  long port = strtol(argv[1], &p, 10);
  if (errno != 0 || *p != '\0' || port > INT_MAX || port < INT_MIN) {
    fprintf(stderr, "Port number is invalid!\n");
    exit(EXIT_FAILURE);
  }

  // initialize OpenSSL
  SSL_CTX* ssl_ctx;
  ssl_ctx = initializeSSL();

  // network functions need arguments in network byte order instead of host byte order.
  // the macros htonl and htons convert the value
  int sockfd, maxfd, retval;
  if ((maxfd = sockfd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
    perror("socket()");
    exit(EXIT_FAILURE);
  }

  memset(&server, 0, sizeof(server));
  server.sin_family = AF_INET;
  server.sin_addr.s_addr = htonl(INADDR_ANY);
  server.sin_port = htons(port);

  if (bind(sockfd, (struct sockaddr*) &server, (socklen_t) sizeof(server)) < 0) {
    perror("bind()");
    exit(EXIT_FAILURE);
  }

  // before the server can accept messages, it has to listen to the welcome port.
  // a backlog of 1 connection is allowed
  if (listen(sockfd, 1) < 0) {
    perror("listen()");
    exit(EXIT_FAILURE);
  }

  // initialize the file descriptor set
  fd_set fdset, tempset;
  FD_ZERO(&fdset);
  FD_SET(sockfd, &fdset);
  struct timeval timeout;
  user_t = g_tree_new((GCompareFunc)sockaddr_in_cmp);

  chatroom_t = g_tree_new((GCompareFunc)g_strcmp0);
  struct chatroom_s* lobby = g_new0(struct chatroom_s, 1);
  lobby->name = LOBBY;
  g_tree_insert(chatroom_t, lobby->name, lobby);
  
  createInitialLog();

  for (;;) {
    // copy the fdset to the temp variable tempset
    memcpy(&tempset, &fdset, sizeof(tempset));

    // initialize the timeout data structure
    timeout.tv_sec = 5;
    timeout.tv_usec = 0;

    // block the program until input or output is ready on a specified 
    // set of file descriptors, or until a timer expires, whichever comes first.
    if ((retval = select(maxfd+1, &tempset, NULL, NULL, &timeout)) == -1) {
      perror("select()");
      exit(EXIT_FAILURE);
    } else if (retval > 0) {
      // check if filedes is a member of the file descriptor set set. 
      if (FD_ISSET(sockfd, &tempset)) {
        // accept a tcp connection.
        // connfd is a fresh handle dedicated to this connection.
        socklen_t len = sizeof(client);
        int connfd;
        if ((connfd = accept(sockfd, (struct sockaddr*) &client, &len)) < 0) {
          perror("accept()");
          exit(EXIT_FAILURE);
        }

        printf("Connection established\n");

        // insert into the set
        FD_SET(connfd, &fdset);
        // FD_CLR(sockfd, &tempset);
        maxfd = maxfd < connfd ? connfd : maxfd;

        SSL* ssl = SSL_new(ssl_ctx);
        SSL_set_fd(ssl, connfd);
        if (SSL_accept(ssl) < 0) {
          perror("SSL_accept()");
          exit(EXIT_FAILURE);
        }

        char addr[INET_ADDRSTRLEN];
        getClientAddr(client, addr);
        int clientPort = client.sin_port;

        // insert user into the tree
        struct user_s* user = g_new0(struct user_s, 1);
        user->fd = connfd;
        user->ssl = ssl;
        user->username = NULL;
        user->chatroom = LOBBY;
        user->ip = addr;
        user->port = port;

        lobby->list = g_list_append(lobby->list, &client);

        g_tree_insert(user_t, &client, user);

        // send the message
        if (SSL_write(ssl, WELCOME, sizeof(WELCOME)) == -1) {
          printf("%d\n", SSL_get_error(ssl, -1));
          printf("Unable to send due to:\n%s\n", ERR_error_string(ERR_get_error(), NULL));
        }

        createRequestLog(addr, clientPort, "connected");
      }

      g_tree_foreach(user_t, (GTraverseFunc)traverseTree, &tempset);
    }

    // checkconnections
  }

  close(sockfd);
  SSL_CTX_free(ssl_ctx);

  return 0;
}
