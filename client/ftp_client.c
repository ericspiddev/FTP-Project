#include "../common/common.h"

#define SHELL_PROMPT "> "
#define NUMOFCMDS  6

#define DELIM   "\n"
#define CMDDELIM " "

#define MAXCLIENTS 10

void userInterface();
int randomizeDataPort();
int dataServer();
int initDataSocket(int dataPort);
ftpErrors ftpHelp();
ftpErrors ftpConnect(char* ip, char* port, int sock);
ftpErrors ftpList(int welcomeSocket, int dataPort);
ftpErrors ftpRetr(char* fileName, int welcomeSocket, int dataPort);
ftpErrors ftpStor(char* fileName, int welcomeSocket, int dataPort);
ftpErrors ftpQuit(void);
ftpErrors stringToUpper(char* string);
ftpErrors getUserInput(char* string);
ftpErrors validateUserCommand(char* string);
ftpErrors executeUserCommand(char* baseCmd, char* fullCommand, int commSock, int welcomeSock, int dataPort, bool* isConnected);
char* commands[NUMOFCMDS] = {"CONNECT", "LIST", "RETR", "STOR", "QUIT", "HELP"};

int main()
{
    int controlSocket, dataSocket;
    bool isConnected = false;
    char usrCommand[MAX_STR_SIZE];
    char cpyUsrCommand[MAX_STR_SIZE];
    char firstCommand[MAX_STR_SIZE];

    srand(time(NULL));

    userInterface();
    controlSocket = socket(AF_INET, SOCK_STREAM, 0);
    int welcomeSocket = 0;
    int dataPort = 0;
    do{
        dataPort = (rand() % 65535) + 1000; // randomize port number for data connections
        welcomeSocket = initDataSocket(dataPort);
    }while(welcomeSocket < 0);
    while(1){
        ftpErrors err = success;
        printf(SHELL_PROMPT);
        if(getUserInput(usrCommand) != success){
            printf("Please enter a non empty command!\n");
            continue;
        }
        strcpy(cpyUsrCommand, usrCommand);
        strcpy(firstCommand, strtok(cpyUsrCommand, CMDDELIM));
        if(stringToUpper(firstCommand) != success){
            printf("Null string detected let's try that again\n");
            continue;
        }
        if(validateUserCommand(firstCommand) != success){
            printf("Please enter a valid FTP command use HELP to see valid commands\n");
            continue;
        }
        if(executeUserCommand(firstCommand, usrCommand, controlSocket, welcomeSocket, dataPort, &isConnected) != success){
            continue;
        }
        clearString(firstCommand);
        clearString(usrCommand);
    }
}

void userInterface()
{
    printf("**************************FTP Client**************************\n");
    printf("Enter a  FTP command\n");
    printf("For a list of commands type \"HELP\"\n");
}


ftpErrors getUserInput(char* string)
{
    if(strcmp(fgets(string, MAX_STR_SIZE, stdin), "\n") == 0){ // if we get nothing back at all
        return nullString;
    }

    string = strtok(string, DELIM); // remove trailing newline
    if(string == NULL){  // double check our string is okay
        return nullString;
    }
    return success;
}

ftpErrors validateUserCommand(char* string)
{
    char* copyStr = malloc(strlen(string));
    if(copyStr == NULL){
        perror("Failed to allocate string!");
        exit(1);
    }
    strcpy(copyStr, string);
    char* potentialCmd = strtok(string, CMDDELIM);
    for(int cmdIndex = 0; cmdIndex < NUMOFCMDS; cmdIndex++)
    {
        if(strcmp(potentialCmd, commands[cmdIndex]) == 0)
        {
            free(copyStr);
            return success;
        }
    }
    free(copyStr);
    return invalidCommand;
}

ftpErrors executeUserCommand(char* baseCmd, char* fullCommand, int commSock, int welcomeSock, int dataPort, bool* isConnected)
{
    char cpyCmd[strlen(fullCommand)];
    strcpy(cpyCmd, fullCommand);
    strtok(cpyCmd, CMDDELIM);
    ftpErrors err = success;

    if(strcmp(baseCmd, "CONNECT") == 0)
    {
        char* ip = strtok(NULL, CMDDELIM);
        char* port = strtok(NULL, CMDDELIM);
        if(ip == NULL || port == NULL){
            printf("Error in connect command format should be CONNECT <ip> <port>\n");
            err = improperCommand;
            goto returnVal;
        }
        err = ftpConnect(ip, port, commSock);
        if(err != success){
            printf("Error establishing connection!\n");
            *isConnected = false;
            goto returnVal;
        }
        *isConnected = true;
    }
    else if(strcmp(baseCmd, "HELP") == 0){
        err = ftpHelp();
        goto returnVal;
    }
    else if(*isConnected == false){
        printf("The requested command must first establish a connection\n");
        err =  noConnection;
        goto returnVal;
    }
    else if(strcmp(baseCmd, "LIST") == 0){
        sendStr(commSock, baseCmd);
        sendData(commSock, &dataPort, sizeof(dataPort));
        err = ftpList(welcomeSock, dataPort);
    }
    else if(strcmp(baseCmd, "RETR") == 0){
        char* fileName = strtok(NULL, CMDDELIM);
        if(fileName == NULL){
            printf("Please provide a filename to retrieve\n");
            err = fileNameIsNull;
            goto returnVal;
        }
        sendStr(commSock,baseCmd);
        sendData(commSock, &dataPort, sizeof(dataPort));

        err = ftpRetr(fileName, welcomeSock, dataPort);
    }
    else if(strcmp(baseCmd, "STOR") == 0){
        char* fileName = strtok(NULL, CMDDELIM);
        if(fileName == NULL){
            printf("Please provide a filename to store\n");
            err = fileNameIsNull;
            goto returnVal;
        }
        if(open(fileName, O_RDONLY) == -1)
        {
            printf("Requested file to store does not exist\n");
            return fileDoesNotExist;
        }
        sendStr(commSock, baseCmd);
        sendData(commSock, &dataPort, sizeof(dataPort));

       err = ftpStor(fileName, welcomeSock, dataPort);

    }
    else if(strcmp(baseCmd, "QUIT") == 0){
        printf("Quitting FTP Client!\n");
        sendStr(commSock, baseCmd);
        close(commSock);
        close(welcomeSock);
        exit(0);
    }
    else{
        err = invalidCommand;
        goto returnVal;
    }
    returnVal:
    if(err != success){
        printErrors(err);
    }
        return err;
}

ftpErrors stringToUpper(char* string)
{
    if(string == NULL)
        return nullString;
    for(int i = 0; i < strlen(string); i++)
        string[i] = toupper(string[i]); // replace current index with uppercase if necessary
    return success;
}

ftpErrors ftpHelp()
{
    printf("A * next to command indicates that command only works when connected to a FTP server\n");
    printf("CONNECT  <server/name/IP address> <server port> -- connect to a FTP server \n");
    printf("LIST(*) -- list the files located on\n");
    printf("RETR(*) <filename> -- retrieve the specified file from the server\n");
    printf("STOR(*) <filename> -- store the specified file on the server\n");
    printf("QUIT(*) -- terminate the current FTP connection\n");
    printf("HELP -- list all the possible commands for FTP\n");
    return success;
}

ftpErrors ftpConnect(char* ip, char* port, int sock) // connection command
{
    struct sockaddr_in controlAddr;
    controlAddr.sin_family = AF_INET;
    controlAddr.sin_addr.s_addr = inet_addr(ip);
    controlAddr.sin_port = htons(atoi(port));
    int connectStatus = connect(sock, (struct sockaddr *) &controlAddr, sizeof(controlAddr));
    if(connectStatus != 0)
    {
        printf("Connection failed!\n");
        return failedConnection;
    }
    return success;
}

ftpErrors ftpList(int welcomeSocket, int dataPort) // list command
{
    printf("client list\n");
    int dataSocket= dataServer(welcomeSocket, dataPort);
    if(dataSocket < 0)
    {
        printf("Data server setup failed!");
    }
    char fileName[MAX_STR_SIZE];
    do{
        clearString(fileName);
        recvStr(dataSocket ,fileName);
        if(strcmp(fileName, LIST_EOF) == 0){
            break;
        }
        else{
            printf("%s", fileName);
        }
    }while(1);

    close(dataSocket);
    return success;
}

ftpErrors ftpRetr(char* fileName, int welcomeSocket, int dataPort) // retreive command
{
    if(fileName == NULL){
        return fileNameIsNull;
    }

    int dataSocket = dataServer(welcomeSocket, dataPort);  // setup connection
    sendStr(dataSocket, fileName); // send the filename we want to server
    char errorCheck[20] = {'\0'};
    recvStr(dataSocket, errorCheck);
    if(strcmp(errorCheck, "FILEFOUND") == 0){
        FILE* retrFile = fopen(fileName, "w");
        long fileSize = 0;
        recvData(dataSocket, &fileSize, sizeof(fileSize)); // read in size of file
        recvFileOverSocket(dataSocket, retrFile, fileSize, CHUNK_SIZE); // read in file
        fclose(retrFile);
    }
    else{
        printf("Filename %s not found on file server\n", fileName);
    }
    close(dataSocket);
    return success;
}

ftpErrors ftpStor(char* fileName, int welcomeSocket, int dataPort) // store command
{
    if(fileName == NULL){
        return fileNameIsNull;
    }
    int dataSocket = dataServer(welcomeSocket, dataPort);
    int f = -1;
    struct stat fs;

    f = open(fileName, O_RDONLY);
    if(fstat(f, &fs) < 0){
        perror("File stat failed!");
        return fileDoesNotExist;
    }

    sendStr(dataSocket, fileName); // send the file's name
    sendData(dataSocket, &fs.st_size, sizeof(fs.st_size)); // send the size of the file
    sendFileOverSocket(dataSocket, fileName, f, fs.st_size, CHUNK_SIZE); // send the file data

    close(f);
    close(dataSocket);
    return success;
}

ftpErrors ftpQuit() // quit command
{
    printf("Quitting FTP Client...\n");
    return success;
}

int initDataSocket(int dataPort)
{
    int err = 0;

    struct sockaddr_in config;
    config.sin_family = AF_INET;
    config.sin_port = htons(dataPort);
    inet_pton(AF_INET, "127.0.0.1", &config.sin_addr);
    int welcomeSocket = socket(AF_INET, SOCK_STREAM, 0); // 1. Create socket
    if(welcomeSocket < 0){
        perror("Failed to create socket!");
        return -1;
    }
    // 2. Set socket options to allow multiple sockets
    int optionStatus = setsockopt(welcomeSocket, SOL_SOCKET, SO_REUSEADDR, &(int){1}, sizeof(int)); // allow socket to be reused
    if(optionStatus < 0)
    {
        printf("Socket option failed!");
        return -1;
    }
    // 3. Convert ip string to ip address and set config struct ( also set the port and connection type)

    // 4. Bind to the port and ip address provided by the configuration
    err = bind(welcomeSocket,(struct sockaddr *)&config, sizeof(config));
    if(err < 0){
        perror("Init Data Socket Bind failed!");
        return -1;
    }

    err = listen(welcomeSocket, 10);
    if(err < 0){
        perror("Listen failed!");
        return -1;
    }

    return welcomeSocket;
}

int dataServer(int welcomeSocket, int dataPort)
{
    struct sockaddr_in config;
    config.sin_family = AF_INET;
    config.sin_port = htons(dataPort);
    inet_pton(AF_INET, "127.0.0.1", &config.sin_addr);

    // 6. Accept the incoming connection and then return the communication socket
    int length = sizeof(config);
    int dataSock = -1;
    do{
        dataSock = accept(welcomeSocket, (struct sockaddr*)&config, &length);
    }while(dataSock == -1);
    return dataSock;
}