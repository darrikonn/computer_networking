#include "httpd.h"

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

        // send the message back
        send(connfd, message, (size_t) n, 0);

        // close the connection
        shutdown(connfd, SHUT_RDWR);
        close(connfd);
    }

	return 0;
}












