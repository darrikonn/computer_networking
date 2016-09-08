#include "tftpd.h"

const int PACKET_SIZE = 516;
const int MAX_FILES_IN_DIRECTORY = 20;

/****** Operation code ******
 *  2 bytes
 * +--------+
 * | OpCode |
 * +--------+
 */

/*
 * Get the operation code of the package
 */ 
short getOperationCode(char* msg) {
    // converting from network byte order to host byte order
    return ntohs(*(short*) msg);
}

/*
 * Set the operation code to a package
 */
void setOperationCode(char* pkt, short opCode) {
    // cast operation code to char* to be able to append into the message
    char* c_opCode = (char*) &opCode;
    strncpy(pkt, c_opCode, 2);
}

/****** File name ******
 *  2 bytes     char*
 * +--------+----------+
 * | OpCode | FileName |
 * +--------+----------+
 */
/*
 * Get the file name from the package
 */
char* getFileName(char* msg) {
    return msg + 2;
}

/*
 * Validate that the file exists
 */
int validateFileExistance(char* validFiles[20], char* fileName) {
    int i;
    for (i = 0; validFiles[i]; i++) {
        if (strcmp(validFiles[i], fileName) == 0) {
            return 1;
        }
    }
    return 0;
}

/****** Error code and error message ******
 *  2 bytes    2 bytes     char*
 * +--------+-----------+----------+
 * | OpCode | ErrorCode | ErrorMsg |
 * +--------+-----------+----------+
 */
/*
 * Sets the error code and the message
 */
void setErrorCodeAndMessage(char* pkt, short errCode) {
    // cast the error code to char* to be able to append into the message
    char* c_errCode = (char*) &errCode;
    strncpy(pkt+2, c_errCode, 2);

    switch (errCode) {
        case 0:
            strcpy(pkt+4, "Not defined, see error message (if any).");
            break;
        case 1:
            strcpy(pkt+4, "File not found.");
            break;
        case 2:
            strcpy(pkt+4, "Access violation.");
            break;
        case 3:
            strcpy(pkt+4, "Disk full or allocation exceeded");
            break;
        case 4:
            strcpy(pkt+4, "Illegal TFTP operation");
            break;
        case 5:
            strcpy(pkt+4, "Unknown transfer ID.");
            break;
        case 6:
            strcpy(pkt+4, "File already exists.");
            break;
        case 7:
            strcpy(pkt+4, "No such use.");
            break;
    }
}

int main(int argc, char *argv[]) {
    // validate and parse parameters
    if (argc != 3) {
        fprintf(stderr, "Usage: %s <port> <directory>\n", argv[0]);
        exit(-1);
    }

    // string to long(string, endptr, base)
    char* p;
    long port = strtol(argv[1], &p, 10);
    if (errno != 0 || *p != '\0' || port > INT_MAX || port < INT_MIN) {
        fprintf(stderr, "Port number is invalid!\n");
        exit(-1);
    }

    // open directory
    struct dirent *dir_instance;
    DIR* directory = opendir(argv[2]);
    if (directory == NULL) {
        fprintf(stderr, "Could not open directory! Check directory path.\n");
        exit(-1);
    }

    // by opening the directory, I can secure that the client can't ask for data in 
    // parent directories; e.g. with ../../password.txt
    char* validFileNames[MAX_FILES_IN_DIRECTORY];
    char* fileName;
    int fileCnt = 0;
    while ((dir_instance = readdir(directory)) != NULL) {
        fileName = dir_instance->d_name;
        if (fileName[0] != '.') {
            validFileNames[fileCnt++] = dir_instance->d_name;
        }
    }
    closedir(directory);

    int sockfd;
    struct sockaddr_in server, client;
    char messageReceived[PACKET_SIZE], messageComposed[PACKET_SIZE], addr[INET_ADDRSTRLEN];

    // create and bind an UDP socket
    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    memset(&server, 0, sizeof(server));
    server.sin_family = AF_INET;

    server.sin_addr.s_addr = htonl(INADDR_ANY);
    server.sin_port = htons(port);
    bind(sockfd, (struct sockaddr *) &server, (socklen_t) sizeof(server));

    for (;;) {
        socklen_t len = (socklen_t) sizeof(client);
        // ekki minus 1?
        ssize_t n = recvfrom(sockfd, messageReceived, sizeof(messageReceived) - 1, 0, 
                (struct sockaddr *) &client, &len);
        messageReceived[n] = '\0';

        switch(getOperationCode(messageReceived)) {
            case 1: // Read request (RRQ)
                // get the ip address of the client on human readable form
                inet_ntop(AF_INET, &(client.sin_addr), addr, INET_ADDRSTRLEN);
                printf("File \"%s\" requested from %s:%d\n", 
                        getFileName(messageReceived),
                        addr,
                        client.sin_port);
                // need to validate that this file exists in the directory
                fileName = getFileName(messageReceived);
                if (!validateFileExistance(validFileNames, fileName)) {
                    setOperationCode(messageComposed, 5);
                    setErrorCodeAndMessage(messageComposed, 4);
                    printf("%s\n", messageComposed);
                    exit(1);
                    sendto(sockfd, messageComposed, PACKET_SIZE, 0,
                            (struct sockaddr*) &client, len);
                    break;
                }
                
                sendto(sockfd, messageReceived, (size_t) n, 0, 
                        (struct sockaddr*) &client, len);
                break;
            case 2: // Write request (WRQ)
                break;
            case 3: // Data (DATA)
                break;
            case 4: // Acknowledgment (ACK)
                break;
            case 5: // Error (ERROR)
                break;
        }

    }


	return 0;
}
