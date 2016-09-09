#ifndef TFTPD_H_    // Include guard
#define TFTPD_H_

/*
 * Project includes
 */
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <limits.h>
#include <errno.h>

/*
 * Functions
 */
short getOperationCode(char* msg);
void setOperationCode(char* pkt, short opCode);
char* getFileName(char* msg);
short validateFileExistance(char* validFiles[20], char* fileName);
short validateFileViolation(char* fileName);
void setErrorCodeAndMessage(char* pkt, short errCode);
unsigned short getBlockNumber(char* pkt); 
void setMode(char* pkt, int fileNameLength, char* mode);
size_t setData(char* pkt, FILE* fp); 

#endif // TFTPD_H_
