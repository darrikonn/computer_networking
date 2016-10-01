#include "httpd.h"

/*
 * Construct a hash table from the header
 */
void constructHashTable(GHashTable* hash, char* message) {
    gchar** header = g_strsplit(message, "\r\n", -1);

    short isData = 0;
    for (int i = 0; header[i] != '\0'; i++) {
        if (!strcmp(header[i], "")) { // check if header is empty, then the next header is the data
            isData = 1;
            continue;
        } else if (isData) { // insert the data into the hash table
            g_hash_table_insert(hash, g_strdup("data"), 
                    g_strndup(header[i], strlen(header[i])));
        } else {
            gchar** temp = !i 
                ? g_strsplit(header[i], " ", -1)
                : g_strsplit(header[i], ": ", 2);

            if (temp != NULL && temp[0] != NULL && temp[1] != NULL) {
                if (!i) { // check if the header is type of request
                    // get type of request
                    g_hash_table_insert(hash, g_strdup("request-type"), 
                            g_strndup(temp[0], strlen(temp[0])));
                    // get url
                    g_hash_table_insert(hash, g_strdup("url"), 
                            g_ascii_strdown(temp[1], strlen(temp[1])));
                } else {
                    // insert header instance into hash table
                    g_hash_table_insert(hash, g_ascii_strdown(temp[0], strlen(temp[0])), 
                            g_ascii_strdown(temp[1], strlen(temp[1]))); 
                }
            }
            g_strfreev(temp);
        }
    }

    g_strfreev(header);
}

/*
 * Generate post data, if it's a post request
 */
int getPostData(char* postData, GHashTable* hash, RequestType type) {
    if (type == POST) {
        char* data = g_hash_table_lookup(hash, "data");
        // prevent XSS
        if (strstr(data, "<script") ||
                strstr(data, "<SCRIPT")) return 0;

        g_snprintf(postData, DATA_SIZE, "%s%s%s",
                "<h3>Post data:</h3><p>",
                (char*)g_hash_table_lookup(hash, "data"),
                "</p>");
    }

    return 1;
}

/*
 * Generate the html
 */
int generateHTML(GHashTable* hash, char* html, char* addr, int port, RequestType type) {
    char postData[DATA_SIZE];
    initializeArray(postData, DATA_SIZE);
    if (!getPostData(postData, hash, type)) {
        return generateErrorHTML(html, "403 Forbidden");
    }

    return g_snprintf(html, HTML_SIZE, "%s%s%s %s:%d%s%s",
            "<!DOCTYPE html><html><head><meta charset=\"UTF-8\"><title>HTTP server</title></head><body><p>", 
            (char*)g_hash_table_lookup(hash, "host"),
            (char*)g_hash_table_lookup(hash, "url"),
            addr, port,
            postData,
            "</p></body></html>");
}

/*
 * Generate html error message
 */
int generateErrorHTML(char* html, char* errorMessage) {
    return g_snprintf(html, HTML_SIZE, "%s%s%s",
            "<!DOCTYPE html><html><head><meta charset=\"UTF-8\"><title>HTTP server</title></head><body><h1>", 
            errorMessage,
            "</h1></body></html>");
}

/*
 * Generate the server response
 */
void generateServerResponse(char* msg, char* date, char* html, int contentLength, RequestType type) {
    int len = g_snprintf(msg, MESSAGE_SIZE, "%s\n%s%s\n%s\n%s%s\n%s\n%s\n", 
            "HTTP/1.1 200 OK",
            "Date: ", date,
            "Server: Apache",
            "Last-Modified: ", date,
            "Accept-Ranges: bytes",
            "Vary: Accept-Encoding");

    if (type != HEAD) {
        msg += g_snprintf(msg+len, MESSAGE_SIZE, "%s%d\n%s\n\n%s", 
                "Content-Length: ", contentLength,
                "Content-Type: text/html",
                html);
    }
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
 * Get the address of the client
 */
void getClientAddr(struct sockaddr_in client, char* addr) {
    // get the ip address of the client on human readable form
    inet_ntop(AF_INET, &(client.sin_addr), addr, INET_ADDRSTRLEN);
}

/*
 * Initialize the array with known values, i.e. 0
 */
void initializeArray(char* arr, int size) {
    memset(arr, '\0', size);
}

/*
 * Get the actual type of the request, and return as enum request type
 */
RequestType getRequestType(char* type) {
    if (type == NULL) {
        return ERROR;
    } else if (!strcmp(type, "GET")) {
        return GET;
    } else if (!strcmp(type, "POST")) {
        return POST;
    } else if (!strcmp(type, "HEAD")) {
        return HEAD;
    }

    return ERROR;
}

/*
 * Create the initial log message
 */
void createInitialLog() {
    char date[DATE_SIZE];
    getCurrentDate(date);
    char message[LOG_MESSAGE_SIZE];
    g_snprintf(message, LOG_MESSAGE_SIZE, "%s -> Starting to log:\n%s", 
            date, "=================================================");

    logToFile(message);
}

/*
 * Create the log message for all requests
 */
void createRequestLog(char* date, char* addr, int port, GHashTable* hash, int resp) {
    char message[LOG_MESSAGE_SIZE];
    g_snprintf(message, LOG_MESSAGE_SIZE, "%s : %s:%d %s %s : %d",
            date, addr, port,
            (char*)g_hash_table_lookup(hash, "request-type"), 
            (char*)g_hash_table_lookup(hash, "url"), resp);

    logToFile(message);
}

/*
 * Log the message to the log file 
 */
void logToFile(char* message) {
    // open file in appending mode
    FILE* f = fopen("log/httpd.log", "a");
    if (f == NULL) {
        perror("fopen()");
        exit(EXIT_FAILURE);
    }
    fprintf(f, "%s\n", message);
    fclose(f);
}

/*
 * Check for connections that are past the connection timout
 */
void checkConnectionTimeouts(time_t* clientsTimeOuts, size_t ctoSize, fd_set* fdset) {
    for (size_t i = 0; i < ctoSize; i++) {
        double t;
        if ((t = clientsTimeOuts[i]) != -1) {
            time_t now;
            time(&now);
            if (difftime(now, t) > CONNECTION_TIME_OUT) {
                // close the connection
                closeConnection(clientsTimeOuts, i, fdset);
            }
        }
    }
}

/*
 * Close the connection
 */
void closeConnection(time_t* clientsTimeOuts, int connfd, fd_set* fdset) {
    printf("Shutting down\n");
    shutdown(connfd, SHUT_RDWR);
    close(connfd);
    clientsTimeOuts[connfd] = -1;
    FD_CLR(connfd, fdset);
}

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
    char message[MESSAGE_SIZE], addr[INET_ADDRSTRLEN];
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

    int retval, maxfd = sockfd;

    // initialize the file descriptor set
    fd_set fdset, tempset;
    FD_ZERO(&fdset);
    FD_SET(sockfd, &fdset);
    struct timeval timeout;

    // initialize the client timeouts
    time_t clientsTimeOuts[MAX_CONNECTIONS];
    memset(clientsTimeOuts, -1, sizeof(clientsTimeOuts));
    size_t ctoSize = sizeof(clientsTimeOuts)/sizeof(time_t);

    // create initial log
    createInitialLog();

    for (;;) {
        // check for inactivity of file descriptors
        checkConnectionTimeouts(clientsTimeOuts, ctoSize, &fdset);

        // copy the fdset to the temp variable tempset
        memcpy(&tempset, &fdset, sizeof(tempset));
        
        // initialize the timeout data structure
        timeout.tv_sec = 3;
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
                socklen_t len = (socklen_t) sizeof(client);
                int connfd;
                if ((connfd = accept(sockfd, (struct sockaddr*) &client, &len)) < 0) {
                    perror("accept()");
                    exit(EXIT_FAILURE);
                }
                // start timing the connection
                time(&clientsTimeOuts[connfd]);

                // insert into the set
                FD_SET(connfd, &fdset);
                FD_CLR(sockfd, &tempset);
                maxfd = maxfd < connfd ? connfd : maxfd;
            }

            for (int i = 0; i <= maxfd; i++) {
                // check if filedes is a member of the file descriptor set set. 
                if (!FD_ISSET(i, &tempset)) continue;
                time(&clientsTimeOuts[i]);

                ssize_t n = recv(i, message, sizeof(message)-1, 0);
                message[n] = '\0';

                // construct a hash table from the header
                GHashTable* hash = g_hash_table_new_full(g_str_hash,    // hash function
                        g_str_equal,   // operator function
                        g_free,        // key destructor
                        g_free);       // value destructor
                constructHashTable(hash, message);

                // get clients address on human readable form
                getClientAddr(client, addr);
                int clientPort = client.sin_port;

                // get type of request
                char* typeOfRequest = g_hash_table_lookup(hash, "request-type");
                RequestType type = getRequestType(typeOfRequest);

                fprintf(stdout, "Received:\n%s request from %s:%d\n", 
                        typeOfRequest, addr, clientPort);

                // initialize the response message
                char responseMessage[MESSAGE_SIZE];
                initializeArray(responseMessage, MESSAGE_SIZE);

                // get the current date
                char date[DATE_SIZE];
                getCurrentDate(date);

                char html[HTML_SIZE];

                if (type == ERROR) {
                    // not implemented
                    int contentLength = generateErrorHTML(html, "400 Bad Request");
                    generateServerResponse(responseMessage, date, html, contentLength, type);
                } else if (type == HEAD) {
                    generateServerResponse(responseMessage, date, NULL, 0, type);
                } else { // type = POST | GET
                    int contentLength = generateHTML(hash, html, addr, clientPort, type);
                    generateServerResponse(responseMessage, date, html, contentLength, type);
                }

                // log the request
                createRequestLog(date, addr, clientPort, hash, 200);

                // send the message back
                send(i, responseMessage, MESSAGE_SIZE, 0);

                // check for persistant connection
                char* connection = g_hash_table_lookup(hash, "connection");
                if (connection == NULL || strcmp(connection, "keep-alive")) {
                    // close the connection
                    closeConnection(clientsTimeOuts, i, &fdset);
                }

                // destroy the hash table
                g_hash_table_destroy(hash);
            }
        }
    }

	return 0;
}
