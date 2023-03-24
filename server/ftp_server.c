#include "../common/common.h"

void list();
ftpErrors retr();
void store();
void quit();
int dataClient();

#define COM_PORT 5665

int main()
{
    int status;
    int welcomeSocket, dataSocket;
    struct sockaddr_in controlSocketConfig;
    welcomeSocket = socket(AF_INET, SOCK_STREAM, 0);

    if(welcomeSocket < 0){
        perror("Socket creation failed!");
        exit(0);
    }

    int optionStatus = setsockopt(welcomeSocket, SOL_SOCKET, SO_REUSEADDR, &(int){1}, sizeof(int)); // allow socket to be reused
    if(optionStatus < 0)
    {
        printf("Option failed!");
        exit(1);
    }

    controlSocketConfig.sin_family = AF_INET;
    controlSocketConfig.sin_port = htons(COM_PORT);
    inet_pton(AF_INET, LOCALHOST, &controlSocketConfig.sin_addr);
    status = bind(welcomeSocket, (struct sockaddr*) &controlSocketConfig, sizeof(controlSocketConfig));
    if(status < 0){
        perror("Bind failed!");
        exit(1);
    }
    status = listen(welcomeSocket, 10);
    if(status < 0){
        perror("Listen failed!");
        exit(1);
    }
    int length = sizeof(controlSocketConfig);
    int controlSocket = accept(welcomeSocket,  (struct sockaddr* )&controlSocketConfig, &length);
    if(controlSocket < 0){
        perror("Failed to accept connection!");
        exit(1);
    }
    char receiveStr[50] = {'\0'};
    while(1){

        clearString(receiveStr);
        recvStr(controlSocket, receiveStr);

        if(strstr(receiveStr, "LIST") != NULL){
            printf("Executing LIST command\n");
            list();
        }

        else if(strstr(receiveStr, "STOR") != NULL){
            printf("Executing STOR command\n");
            store();
        }

        else if(strstr(receiveStr, "RETR") != NULL){
            printf("Executing RETR command\n");
            retr();
        }

        else if(strstr(receiveStr, "QUIT")){
            printf("Quitting FTP Server!\n");
            close(controlSocket);
            exit(1);
        }
    }

    return 0;
}

void list(){
    printf("Inside server list\n");
    int dataSocket = dataClient();
    char fileName[MAX_STR_SIZE] = {'\0'};
    if(dataSocket < 0){
        printf("Failed to connect to data server!\n");
        exit(1);
    }
    DIR *d;
    struct dirent *dir;
    d = opendir(".");
    if(d){
        while((dir = readdir(d)) != NULL)
        {
            sprintf(fileName, "%s\n", dir->d_name);
            sendStr(dataSocket, fileName);
            clearString(fileName);
        }
        sendStr(dataSocket, LIST_EOF);
        closedir(d);
    }
    close(dataSocket);
}

ftpErrors retr(){
    struct stat fs;
    char fileName[MAX_STR_SIZE] = {'\0'};
    int dataSocket = dataClient();
    recvStr(dataSocket, fileName); // recv the name of the file
    int f = open(fileName, O_RDONLY);
    if(f < 0) // the file has not been found
    {
        printf("Requested file to store does not exist\n");
        return fileDoesNotExist;
    }

    if(fstat(f, &fs) < 0){  // we got a syscall error
        perror("File stat failed!");
        return fileDoesNotExist;
    }

    sendData(dataSocket, &fs.st_size, sizeof(fs.st_size)); // send the size of the file
    printf("File size is %ld\n", fs.st_size);
    sendFileOverSocket(dataSocket, fileName, f, fs.st_size, CHUNK_SIZE); // send the file data

    close(f);
    close(dataSocket);
    return success;
}


void store(){
    char fileName[MAX_STR_SIZE] = {'\0'};
    int dataSocket = dataClient();

    recvStr(dataSocket, fileName); // grab the fileName
    FILE* storeFile = fopen(fileName, "w"); // make new file with the sentover name

    long fileSize = 0;
    recvData(dataSocket, &fileSize, sizeof(fileSize)); // grab the fileSize

    recvFileOverSocket(dataSocket, storeFile, fileSize, CHUNK_SIZE);
    fclose(storeFile);
    close(dataSocket);
}

int dataClient(){
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if(sock < 0 )
    {
        perror("Failed to allocate socket!");
    }

    int optionStatus = setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &(int){1}, sizeof(int));
    if(optionStatus < 0){
        perror("Something went wrong setting socket option!");
    }

    struct sockaddr_in controlAddr;
    controlAddr.sin_family = AF_INET;
    controlAddr.sin_addr.s_addr = inet_addr("127.0.0.1");
    controlAddr.sin_port = htons(DATAPORT);
    printf("Execute connect!\n");
    int err = -1;
    do{
        err = connect(sock, (struct sockaddr *) &controlAddr, sizeof(controlAddr));
    }while(err == -1);
    return sock;
}