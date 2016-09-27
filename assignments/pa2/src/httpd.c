#include "httpd.h"

/*
 * Construct a hash table from the header
 */
void constructHashTable(GHashTable* hash, char* message) {
    gchar** header = g_strsplit(message, "\r\n", -1);

    for (int i = 0; header[i] != '\0'; i++) {
        gchar** temp = !i 
            ? g_strsplit(header[i], " ", -1)
            : g_strsplit(header[i], ": ", 2);

        if (temp != NULL && temp[0] != NULL && temp[1] != NULL) {
            // insert type of request
            if (!i) {
                // get type of request
                g_hash_table_insert(hash, g_strdup("request-type"), 
                        g_strndup(temp[0], strlen(temp[0])));
                // get url
                g_hash_table_insert(hash, g_strdup("url"), 
                        g_ascii_strdown(temp[1], strlen(temp[1])));
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
 * Generate post data, if it's a post request
 */
void getPostData(char* postData, GHashTable* hash, RequestType type) {
    if (type == POST) {
        g_snprintf(postData, DATA_SIZE, "%s%s%s",
                "<h3>Post data:</h3><p>",
                (char*)g_hash_table_lookup(hash, "content"),
                "</p>");
    }
}

/*
 * Generate the html
 */
int generateHTML(GHashTable* hash, char* html, char* addr, int port, char* postData) {
    return g_snprintf(html, HTML_SIZE, "%s%s%s %s:%d%s%s",
            "<!DOCTYPE html><html><head><meta charset=\"UTF-8\"><title>HTTP server</title></head><body><p>", 
            (char*)g_hash_table_lookup(hash, "host"),
            (char*)g_hash_table_lookup(hash, "url"),
            addr, port,
            postData,
            "</p></body></html>");
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

    /// setja rett iso stadal a date
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

    // create initial log
    createInitialLog();

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

        if (type == ERROR) {
            // send not implemented
        } else if (type == HEAD) {
            generateServerResponse(responseMessage, date, NULL, 0, type);
        } else {
            char html[HTML_SIZE];
            char postData[DATA_SIZE];
            initializeArray(postData, DATA_SIZE);
            getPostData(postData, hash, type);
            int contentLength = generateHTML(hash, html, addr, clientPort, postData);
            generateServerResponse(responseMessage, date, html, contentLength, type);
        }

        // log the request
        createRequestLog(date, addr, clientPort, hash, 200);

        // send the message back
        send(connfd, responseMessage, MESSAGE_SIZE, 0);

        // destroy the hash table
        g_hash_table_destroy(hash);

        // close the connection
        shutdown(connfd, SHUT_RDWR);
        close(connfd);
    }

	return 0;
}
