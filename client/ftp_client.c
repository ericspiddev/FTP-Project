#include <stdio.h>
#include <stdlib.h>
#include <ctype.h> // toUpper
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>


typedef enum ftpErrors{
    success,
    nullString,
    invalidCommand,
} ftpErrors;

#define SHELL_PROMPT "> "
#define NUMOFCMDS  6
#define MAX_STR_SIZE 256
#define DELIM   "\n"


void ftpHelp();
void ftpConnect(char* ip, char* port);
void ftpList(void);
void ftpRetr(char* fileName);
void ftpStor(char* fileName);
void ftpQuit(void);
void userInterface();
ftpErrors stringToUpper(char* string);
ftpErrors getUserInput(char* string);
ftpErrors validateUserCommand(char* string);
ftpErrors executeUserCommand(char* command);

char* commands[NUMOFCMDS] = {"CONNECT", "LIST", "RETR", "STOR", "QUIT", "HELP"};



int main()
{
    ftpErrors err;
    char usrCommand[MAX_STR_SIZE];
    int clientSocket = socket()

    userInterface();
    while(1){
        printf(SHELL_PROMPT);
        if(getUserInput(usrCommand) != success){
            printf("Please enter a non empty command!\n");
            continue;
        }
        if(stringToUpper(usrCommand) != success){
            printf("Null string detected let's try that again\n");
            continue;
        }
        if(validateUserCommand(usrCommand) != success){
            printf("Please enter a valid FTP command use HELP to see valid commands\n");
            continue;
        }
    }
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
    for(int cmdIndex = 0; cmdIndex < NUMOFCMDS; cmdIndex++)
    {
        char* potentialCmd = strndup(string, strlen(commands[cmdIndex]));
        printf("Potential command  is %s\n", potentialCmd);
        printf("String is %s\n", string);
        if(strcmp(potentialCmd, commands[cmdIndex]) == 0)
        {
            return success;
        }
    }
    return invalidCommand;
}

ftpErrors executeUserCommand(char* command)
{
    // if(strcmp(command, "CONNECT"))
    return success;
}


ftpErrors stringToUpper(char* string)
{
    if(string == NULL)
        return nullString;
    for(int i = 0; i < strlen(string); i++)
        string[i] = toupper(string[i]); // replace current index with uppercase if necessary
    return success;
}

void userInterface()
{
    printf("**************************FTP Client**************************\n");
    printf("Enter a  FTP command\n");
    printf("For a list of commands type \"HELP\"\n");
}


void ftpHelp()
{
    printf("A * next to command indicates that command only works when connected to a FTP server\n");
    printf("CONNECT  <server/name/IP address> <server port> -- connect to a FTP server \n");
    printf("LIST(*) -- list the files located on\n");
    printf("RETR(*) <filename> -- retrieve the specified file from the server\n");
    printf("STOR(*) <filename> -- store the specified file on the server\n");
    printf("QUIT(*) -- terminate the current FTP connection\n");
    printf("HELP -- list all the possible commands for FTP\n");
}

void ftpConnect(char* ip, char* port) // connection command
{
    
}

void ftpList() // list command
{
    return;
}

void ftpRetr(char* fileName) // retreive command
{
    return;
}

void ftpStor(char* fileName) // store command
{
    return;
}

void ftpQuit() // quit command
{
    return;
}