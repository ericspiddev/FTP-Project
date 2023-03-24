#include "./common.h"

void clearString(char* string){
    for(int i =0; i < MAX_STR_SIZE; i++)
    {
        string[i] = '\0';
    }
}

void recvStr(int sock, char* strToFill)
{
    uint32_t strSize = 0;
    recvData(sock, &strSize, sizeof(strSize)); // recv the str's length
    strSize = ntohl(strSize); // reverse byte order
    recvData(sock, strToFill, strSize); // recv actual string
}

void sendStr(int sock, char* strToSend){
    uint32_t sendStrSize = htonl(strlen(strToSend));
    uint32_t strSize = strlen(strToSend);
    sendData(sock, &sendStrSize, sizeof(sendStrSize)); // send the string's size
    sendData(sock, strToSend, strSize); // send the actual string
}

void sendFileOverSocket(int sock, char* fileName, int fileHandle, int fileSize, int chunkSize)
{
    int offset = 0;
    int remainder = fileSize % chunkSize;
    if(remainder != 0){
        printf("Sending remainder data!\n");
        sendFileData(sock, fileHandle, &offset, remainder, remainder);
        fileSize -= remainder;
    }
    if(fileSize >=  0){
        printf("Sending the data in chunks!\n");
        sendFileData(sock, fileHandle, &offset, fileSize, chunkSize);
    }
}

void recvFileOverSocket(int sock, FILE* fileHandle, int fileSize,  int chunkSize)
{
    int remainder = fileSize % chunkSize;
    if(remainder != 0){
        recvFileData(sock, fileHandle, remainder, remainder); // recv data that has a size of remainder and in 1 chunk
        fileSize -= remainder;
    }
    if(fileSize > 0 ){
        recvFileData(sock, fileHandle, fileSize, chunkSize);
    }
}


void sendFileData(int sock, int fileHandle, int* fileOffset, int fileSize, int chunkSize)
{
    int bytesSent = 0;
    do{
        int currSentBytes = sendfile(sock, fileHandle, NULL, chunkSize);
        if(currSentBytes < 0){
            perror("Error sending file!");
        }
        else{
            bytesSent += currSentBytes;
        }
    }while(bytesSent != fileSize);
}

void recvFileData(int sock, FILE* fileHandle, int fileSize, int chunkSize)
{
    int bytesRecv = 0;
    char buf[chunkSize];
    do{
        int currBytesRecv = recv(sock, buf, chunkSize, 0);
        if(currBytesRecv < 0){
            perror("Error receiving file data!");
        }
        else{
            fwrite(buf, sizeof(char), currBytesRecv, fileHandle);
            printf("buf is %s\n", buf);
            bytesRecv +=  currBytesRecv;
            memset(buf, 0, currBytesRecv); // clear the buffer for the next read
            printf("BytesRecv is %d and fileSize is %d\n", bytesRecv, fileSize);
        }
    }while(bytesRecv != fileSize);
}

void recvData(int sock, void* buff, int size){
    int bytesRecv = 0;
    do{
        int currBytesRecv = recv(sock, buff, size, 0);
        if(currBytesRecv < 0){
            perror("Something went wrong receiving bytes!");
        }
        else{
            bytesRecv += currBytesRecv;
        }
    }while(bytesRecv != size);
}

void sendData(int sock, void* buff, int size){
    int bytesSent = 0;
    do{
        int currBytesSent = send(sock, buff, size, 0);
        if(currBytesSent < 0){
            perror("Something went wrong sending bytes!");
        }
        else{
            bytesSent += currBytesSent;
        }
    }while(bytesSent != size);

}