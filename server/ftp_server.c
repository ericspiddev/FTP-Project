#include "../common/common.h"

ftpErrors serverList(int dataPort);
ftpErrors serverRetr(int dataPort);
ftpErrors serverStor(int dataPort);
int dataClient(int dataPort);

void* ftpServer(void* socket);

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

    while(1)
    {
        int controlSocket = accept(welcomeSocket,  (struct sockaddr* )&controlSocketConfig, &length);
        pthread_t serverThread;
        if(controlSocket < 0)
        {
            perror("Failed to accept connection!");
            exit(1);
        }
        else
        {
            pthread_create(&serverThread, NULL, ftpServer, &controlSocket);
            pthread_detach(serverThread);
        }
    }

    return 0;
}

void* ftpServer(void* socket){
    int controlSocket = 0;
    memcpy(&controlSocket, (int*) socket, sizeof(int));

    char receiveStr[MAX_STR_SIZE] = {'\0'};
    char copyCommand[MAX_STR_SIZE] = {'\0'};
    while(1){
        int dataPort = 0;
        ftpErrors err = success;
        clearString(receiveStr);
        clearString(copyCommand);
        recvStr(controlSocket, receiveStr);
        strcpy(copyCommand, receiveStr);
        char* baseCmd = strtok(copyCommand, " ");
        if(strcmp(baseCmd, "LIST") == 0){
            printf("Executing LIST command\n");
            recvData(controlSocket, &dataPort, sizeof(dataPort));
            err = serverList(dataPort);
        }

        else if(strcmp(baseCmd, "STOR") == 0){
            printf("Executing STOR command\n");
            recvData(controlSocket, &dataPort, sizeof(dataPort));
            err= serverStor(dataPort);
        }

        else if(strcmp(baseCmd, "RETR") == 0){
            recvData(controlSocket, &dataPort, sizeof(dataPort));
            printf("Executing RETR command\n");
            err = serverRetr(dataPort);
        }

        else if(strcmp(baseCmd, "QUIT") == 0){
            printf("Quitting FTP Server!\n");
            close(controlSocket);
            pthread_exit(0);
        }
        if(err != success)
            printErrors(err);
    }
}

ftpErrors serverList(int dataPort){
    ftpErrors err = success;
    int dataSocket = dataClient(dataPort);
    char fileName[MAX_STR_SIZE] = {'\0'};
    if(dataSocket < 0){
        printf("Failed to connect to data server!\n");
        pthread_exit(0);
    }
    DIR *d;
    struct dirent *dir;
    d = opendir(".");
    if(d){
        while((dir = readdir(d)) != NULL)
        {
            if(strcmp(dir->d_name, ".") != 0 && strcmp(dir->d_name, "..") != 0){ // eliminate the folders in directory
                sprintf(fileName, "%s\n", dir->d_name);
                sendStr(dataSocket, fileName);
                clearString(fileName);
            }

        }
        sendStr(dataSocket, LIST_EOF);
        closedir(d);
    }
    close(dataSocket);
    return err;
}

ftpErrors serverRetr(int dataPort){
    ftpErrors err = success;
    struct stat fs;
    char fileName[MAX_STR_SIZE] = {'\0'};
    int dataSocket = dataClient(dataPort);
    recvStr(dataSocket, fileName); // recv the name of the file
    int f = open(fileName, O_RDONLY);
    if(f < 0) // the file has not been found
    {
        printf("Requested file to store does not exist\n");
        sendStr(dataSocket, "FILENOTFOUND");
        close(dataSocket);
        err =  fileDoesNotExist;
        return err;
    }

    sendStr(dataSocket, "FILEFOUND");
    if(fstat(f, &fs) < 0){  // we got a syscall error
        perror("File stat failed!");
        err =  fileDoesNotExist;
        return err;
    }

    sendData(dataSocket, &fs.st_size, sizeof(fs.st_size)); // send the size of the file
    sendFileOverSocket(dataSocket, fileName, f, fs.st_size, CHUNK_SIZE); // send the file data
    close(f);
    close(dataSocket);
    return err;
}


ftpErrors serverStor(int dataPort){
    char fileName[MAX_STR_SIZE] = {'\0'};
    int dataSocket = dataClient(dataPort);

    recvStr(dataSocket, fileName); // grab the fileName
    FILE* storeFile = fopen(fileName, "w"); // make new file with the sentover name

    long fileSize = 0;
    recvData(dataSocket, &fileSize, sizeof(fileSize)); // grab the fileSize

    recvFileOverSocket(dataSocket, storeFile, fileSize, CHUNK_SIZE);
    fclose(storeFile);
    close(dataSocket);
}

int dataClient(int dataPort){
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
    controlAddr.sin_addr.s_addr = inet_addr(LOCALHOST);
    controlAddr.sin_port = htons(dataPort);
    int err = -1;
    do{
        err = connect(sock, (struct sockaddr *) &controlAddr, sizeof(controlAddr));
    }while(err == -1);
    return sock;
}