#include "tftpd.h"

/****** Operation code ******
 *  2 bytes
 * +--------+
 * | OpCode |
 * +--------+
 */

/*
 * Get the operation code of the package
 */
short getOperationCode(char* pkt) {
    // returning the first two bytes representation of the packet
    return (pkt[0] << 8 | pkt[1]);
}

/*
 * Set the operation code to a package
 */
void setOperationCode(char* pkt, short opCode) {
    // get 2 byte representation of the operation code
    // insert in the first two slots of the packet
    pkt[0] = (opCode >> 8) & 0xff;
    pkt[1] = opCode & 0xff;
}

/****** File ******
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
short validateFileExistance(char validFiles[MAX_FILES_IN_DIRECTORY][MAX_FILENAME_LENGTH],
                            char* fileName) {
    int i;
    for (i = 0; validFiles[i]; i++) {
        if (strcmp(validFiles[i], fileName) == 0) {
            return 1;
        }
    }
    return 0;
}

/*
 * Validate file violation, if it contains ../ (directory navigation)
 */
short validateFileViolation(char* fileName) {
    return strstr(fileName, "../") == NULL;
}

/****** Error code and error message ******
 *  2 bytes    2 bytes     char*    1 byte
 * +--------+-----------+----------+------+
 * | OpCode | ErrorCode | ErrorMsg |   0  |
 * +--------+-----------+----------+------+
 */

/*
 * Sets the error code and the message
 */
void setErrorCodeAndMessage(char* pkt, short errCode) {
    // get 2 byte representation of the error code
    // insert in the third and fourth slots of the packet
    pkt[2] = (errCode >> 8) & 0xff;
    pkt[3] = errCode & 0xff;

    // next append the error message to the packet
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

    // insert the last 0 byte to the packet
    pkt[PACKET_SIZE-1] = '\0';
}

/****** Block number ******
 *  2 bytes   2 bytes
 * +--------+---------+
 * | OpCode | Block # |
 * +--------+---------+
 */

/*
 * Get the block number from the packet as an unsigned short
 */
unsigned short getBlockNumber(char* pkt) {
    // get 2 byte representation as an unsigned short
    return (((pkt[2] << 8) & 0xff00) | (pkt[3] & 0xff));
}

/*
 * Set the block number to the packet
 */
void setBlockNumber(char* pkt, unsigned short blockNumber) {
    // get 2 byte representation of the block number
    // insert in the third and fourth slots of the packet
    pkt[2] = (blockNumber >> 8) & 0xff;
    pkt[3] = blockNumber & 0xff;
}

/****** File type ******
 *  2 bytes     char*    1 byte  char*
 * +--------+----------+--------+------+
 * | OpCode | fileName |    0   | Mode |
 * +--------+----------+--------+------+
 */

/*
 * Set the read mode of the file being opened
 */
void setMode(char* pkt, int fileNameLength, char* mode) {
    char* modeType = pkt+3+fileNameLength;
    if (!strcmp(modeType, "octet")) {
        strcpy(mode, "rb");
    } else {
        strcpy(mode, "r");
    }
}

/****** Data ******
 *  2 bytes   2 bytes  n bytes
 * +--------+---------+--------+
 * | OpCode | Block # |  Data  |
 * +--------+---------+--------+
 */

/*
 * Add the data from the file to the packet
 */
size_t setData(char* pkt, FILE* fp) {
    // read 512 bytes from the file
    //           ptr, size, nmemb, file
    return fread(pkt+4, 1, 512, fp)+4;
}

int main(int argc, char* argv[]) {
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
    struct dirent* dir_instance;
    char dir_name[20];
    memcpy(dir_name, argv[2], 20); // use memcpy when we know the size (faster)
    DIR* directory = opendir(dir_name);
    if (directory == NULL) {
        fprintf(stderr, "Could not open directory! Check directory path.\n");
        exit(-1);
    }

    // by opening the directory, I can secure that the client can't ask for data in
    // parent directories; e.g. with ../../password.txt
    char validFileNames[MAX_FILES_IN_DIRECTORY][MAX_FILENAME_LENGTH];
    char fileName[MAX_FILENAME_LENGTH];
    int fileCnt = 0;
    while ((dir_instance = readdir(directory)) != NULL) {
        if (dir_instance->d_name[0] != '.') {
            strcpy(validFileNames[fileCnt++], dir_instance->d_name);
        }
    }
    closedir(directory);

    int sockfd;
    struct sockaddr_in server, client;
    char packetReceived[PACKET_SIZE], packetComposed[PACKET_SIZE], addr[INET_ADDRSTRLEN];
    unsigned short nextBlockNumber = 1, prevBlockNumber;
    FILE* fp = NULL;
    size_t data_size = 0;

    // create and bind an UDP socket
    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    memset(&server, 0, sizeof(server));
    server.sin_family = AF_INET;

    // network functions need arguments in network byte order instead of host byte order
    // the macros htonl and htons convert the values
    server.sin_addr.s_addr = htonl(INADDR_ANY);
    server.sin_port = htons(port);
    bind(sockfd, (struct sockaddr*) &server, (socklen_t) sizeof(server));

    for (;;) {
        // receive up to one byte less than declared because it will be NULL-terminated later
        socklen_t len = (socklen_t) sizeof(client);
        ssize_t n = recvfrom(sockfd, packetReceived, sizeof(packetReceived) - 1, 0,
                             (struct sockaddr*) &client, &len);
        // NULL-terminate the packet received
        packetReceived[n] = '\0';

        // get the operation code of the packet
        switch (getOperationCode(packetReceived)) {
            case 1: // Read request (RRQ)
                // fetch file name being requested
                strcpy(fileName, getFileName(packetReceived));

                // validate if the file request contains directory navigation
                if (!validateFileViolation(fileName)) {
                    setOperationCode(packetComposed, 5); // error
                    setErrorCodeAndMessage(packetComposed, 2); // access violation
                    sendto(sockfd, packetComposed, PACKET_SIZE, 0,
                           (struct sockaddr*) &client, len);
                    break;
                }

                // get the ip address of the client on human readable form
                inet_ntop(AF_INET, &(client.sin_addr), addr, INET_ADDRSTRLEN);
                printf("File \"%s\" requested from %s:%d\n", fileName, addr, client.sin_port);

                // be able to read both regular text files and binary files
                char mode[3];
                setMode(packetReceived, strlen(fileName), mode);

                // construct the file location
                char fileLocation[100];
                strcpy(fileLocation, dir_name);
                if (dir_name[strlen(dir_name)-1] != '/') {
                    strcat(fileLocation, "/");
                }
                strcat(fileLocation, fileName);

                // validate that this file exists in the directory
                // if file exists, then check that the file can be opened
                if (!validateFileExistance(validFileNames, fileName) ||
                        (fp = fopen(fileLocation, mode)) == NULL) {
                    setOperationCode(packetComposed, 5); // error
                    setErrorCodeAndMessage(packetComposed, 1); // file not found
                    sendto(sockfd, packetComposed, PACKET_SIZE, 0,
                           (struct sockaddr*) &client, len);
                    break;
                }

                // compose a new packet to send the requested file
                setOperationCode(packetComposed, 3); // data
                // block numbers are consecutive and begin with one
                setBlockNumber(packetComposed, 1);
                data_size = setData(packetComposed, fp);
                // increment the block number
                nextBlockNumber = 2;

                sendto(sockfd, packetComposed, data_size, 0,
                       (struct sockaddr*) &client, len);
                break;
            case 2: // Write request (WRQ) (does not support)
                setOperationCode(packetComposed, 5); // error
                setErrorCodeAndMessage(packetComposed, 4); // illegal operation
                sendto(sockfd, packetComposed, PACKET_SIZE, 0,
                       (struct sockaddr*) &client, len);
                break;
            case 3: // Data (DATA) (does not support)
                setOperationCode(packetComposed, 5); // error
                setErrorCodeAndMessage(packetComposed, 4); // illegal operation
                sendto(sockfd, packetComposed, PACKET_SIZE, 0,
                       (struct sockaddr*) &client, len);
                break;
            case 4: // Acknowledgment (ACK)
                // get the block number of the acknowledged packet
                prevBlockNumber = getBlockNumber(packetReceived);
                if (prevBlockNumber == nextBlockNumber) {
                    // acknowledging error packet being sent from server
                    // do nothing
                } else if (prevBlockNumber == nextBlockNumber - 2) {
                    // a packet loss, so try to send again
                    sendto(sockfd, packetComposed, data_size, 0,
                           (struct sockaddr*) &client, len);
                } else if (prevBlockNumber != nextBlockNumber - 1) {
                    // should not happen, so an error has occured
                    setOperationCode(packetComposed, 5); // error
                    setErrorCodeAndMessage(packetComposed, 5); // unknown transfer ID
                    sendto(sockfd, packetComposed, PACKET_SIZE, 0,
                           (struct sockaddr*) &client, len);
                    if (fp != NULL) {
                        fclose(fp);
                    }
                    fp = NULL;
                } else if (data_size == PACKET_SIZE) { // messages yet to be sent
                    setOperationCode(packetComposed, 3); // data
                    setBlockNumber(packetComposed, nextBlockNumber);
                    data_size = setData(packetComposed, fp);
                    // increment the block number with regards to overflow of unsigned short
                    nextBlockNumber = (nextBlockNumber+1) % USHRT_MAX;

                    sendto(sockfd, packetComposed, data_size, 0,
                           (struct sockaddr*) &client, len);
                } else {
                    // done reading, close the file
                    if (fp != NULL) {
                        fclose(fp);
                    }
                    fp = NULL;
                }
                break;
            case 5: // Error (ERROR)
                // do not retransmit (the TFTP server may terminate)
                break;
            default: // Unknown operation code
                setOperationCode(packetComposed, 5); // error
                setErrorCodeAndMessage(packetComposed, 4); // illegal operation
                sendto(sockfd, packetComposed, PACKET_SIZE, 0,
                       (struct sockaddr*) &client, len);
                break;
        }
    }

    return 0;
}
