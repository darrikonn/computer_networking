#include "httpd.h"

/*
 * Construct a hash table from the header
 */
void constructHashTable(GHashTable* hash, char* msg) {
    gchar** header = g_strsplit(msg, "\r\n", -1);

    for (int i = 0; header[i] != '\0'; i++) {
        gchar** temp = g_strsplit(header[i], !i ? " " : ": ", 2);

        if (temp != NULL && temp[0] != NULL && temp[1] != NULL) {
            // insert type of request
            if (!i) {
                g_hash_table_insert(hash, g_strdup("request-type"), 
                        g_ascii_strdown(temp[0], strlen(temp[0])));
            } else {
                g_hash_table_insert(hash, g_ascii_strdown(temp[0], strlen(temp[0])), 
                        g_ascii_strdown(temp[1], strlen(temp[1]))); 
            }
        }
        g_strfreev(temp);
    }

    g_strfreev(header);
}

/*
 * Generate the html
 */
void generateHTML(GHashTable* hash, char* html) {
    snprintf(html, HTML_SIZE, 
            "<!DOCTYPE html>\
              <html>\
                <head>\
                  <meta charset=\"UTF-8\">\
                  <title>HTTP server</title>\
                </head>\
                <body>\
                  <p>%s</p>\
                </body>\
              </html>", (char*)g_hash_table_lookup(hash, "host"));
    /// nota glib string
}

/*
 * Initialize the array with known values, i.e. 0
 */
void initializeArray(char* arr, int size) {
    memset(arr, '\0', size);
}

/*
void myfun(gpointer key, gpointer val, gpointer u) {
    printf("key: %s\nvalue: %s\n\n", (char*)key, (char*)val);
}
//g_hash_table_foreach(hash, myfun, NULL);
*/

int main(int argc, char *argv[]) {
    // validate and parse parameters
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

    int sockfd;
    char message[MESSAGE_SIZE];
    struct sockaddr_in server, client;

    // create a TCP socket and connect to server
    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("socket()");
        exit(EXIT_FAILURE);
    }

    // network functions need arguments in network byte order instead of host byte order.
    // the macros htonl and htons convert the values
    memset(&server, 0, sizeof(server));
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = htonl(INADDR_ANY);
    server.sin_port = htons(port);

    if (bind(sockfd, (struct sockaddr*) &server, (socklen_t) sizeof(server)) <  0) {
        perror("bind()");
        exit(EXIT_FAILURE);
    }

    // before the server can accept messages, it has to listen to the welcome port.
    // a backlog of one connection is allowed.
    if (listen(sockfd, 1) < 0) {
        perror("listen()");
        exit(EXIT_FAILURE);
    }

    for (;;) {
        // we first have to accept a tcp connection.
        // connfd is a fresh handle dedicated to this connection.
        socklen_t len = (socklen_t) sizeof(client);
        int connfd;
        if ((connfd = accept(sockfd, (struct sockaddr*) &client, &len)) < 0) {
            perror("accept()");
            exit(EXIT_FAILURE);
        }

        // receive from connfd, not sockfd
        ssize_t n = recv(connfd, message, sizeof(message)-1, 0);
        message[n] = '\0';

        fprintf(stdout, "Received:\n%s\n", message);

        // construct a hash table from the header
        GHashTable* hash = g_hash_table_new_full(g_str_hash,    // hash function
                                                 g_str_equal,   // operator function
                                                 g_free,        // key destructor
                                                 g_free);       // value destructor
        constructHashTable(hash, message);

        // get type of request
        char* typeOfRequest = g_hash_table_lookup(hash, "request-type");

        if (typeOfRequest == NULL) {
        } else if (!strcmp(typeOfRequest, "GET")) {
            printf("IIIII'm geeeeeeeeeeeet");
        } else if (!strcmp(typeOfRequest, "POST")) {
        } else if (!strcmp(typeOfRequest, "HEAD")) {
        }

        // send the message back
        char html[HTML_SIZE];
        generateHTML(hash, html);
        send(connfd, html, (size_t) n, 0);

        // destroy the hash table
        g_hash_table_destroy(hash);

        // close the connection
        shutdown(connfd, SHUT_RDWR);
        close(connfd);
    }

	return 0;
}












