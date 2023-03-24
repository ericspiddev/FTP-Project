
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <dirent.h>
#include <stdbool.h>
#include <netdb.h>
#include <ctype.h>
#include <sys/sendfile.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>


typedef enum ftpErrors{
    success,
    nullString,
    invalidCommand,
    noConnection,
    improperCommand,
    failedConnection,
    fileDoesNotExist,
} ftpErrors;

#define LOCALHOST "127.0.0.1"
#define DATAPORT    8080
#define MAX_STR_SIZE 256
#define CHUNK_SIZE  4096

#define LIST_EOF "#ENDOFLISTTRANSMISSION"

void clearString(char* string);
void recvStr(int sock, char* strToFill);
void sendStr(int sock, char* strToSend);

void sendFileOverSocket(int sock, char* fileName, int fileHandle, int fileSize, int chunkSize);
void recvFileOverSocket(int sock, FILE* fileHandle, int fileSize,  int chunkSize);


void sendFileData(int sock, int fileHandle, int* fileOffset, int fileSize, int chunkSize);
void recvFileData(int sock, FILE* fileHandle, int fileSize, int chunkSize);

void recvData(int sock, void* buff, int size);
void sendData(int sock, void* buff, int size);