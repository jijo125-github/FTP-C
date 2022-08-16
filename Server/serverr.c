/*
    How to run Server:
        --> gcc -o tests serverr.c
        --> ./tests "PORTNUMBER" for exmaple --> ./tests 5000
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdbool.h>
#include <errno.h>
#include <sys/dir.h>
#include <time.h>
#include <sys/ipc.h>
#include <sys/shm.h>

#define MAXLINE 1024

int newSocket;
// int count = 0;
char file[1024]; // to store contents from client in server
// char file_name[100];
char pwdbuf[256];
char list[1024];
struct stat st = {0};
char renfr[1024];
bool logged_in = false;

// declaring all the functions
int changeDirectory(char *directory);
int show_currentDirectory();
int ListFilesInDirectory();
int makeDirectory(char *directory);
int write_file(char *filename, int sockfd);
int retr_file(char *filename);
int appe_file(char *filename, char *content);
int abort_service();

int main(int argc, char *argv[])
{
    if (argc != 2)
    {
        printf("error");
    }
    printf("\e]2;FTP-Server\a");

    int ret, portNumber;
    struct sockaddr_in serverAddr; // Server Socket Address Structure
    struct sockaddr_in newAddr;    // Client Socket Address Structure
    socklen_t addr_size;           // Value Result Argument.

    int sockfd;
    char buffer[MAXLINE];
    char buflist[MAXLINE];
    pid_t childpid;

    // socket creation
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0)
    {
        printf("Error in connection.\n");
        exit(1);
    }

    // socket connection
    memset(&serverAddr, '\0', sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;
    sscanf(argv[1], "%d", &portNumber); // IPv4
    serverAddr.sin_port = htons((uint16_t)portNumber);
    serverAddr.sin_addr.s_addr = inet_addr("127.0.0.1"); // Server IP-address of '127.0.0.1'
    printf("Server Socket is created with IP-address %s.\n", inet_ntoa(serverAddr.sin_addr));

    ret = bind(sockfd, (struct sockaddr *)&serverAddr, sizeof(serverAddr)); // binding to the socket

    if (ret < 0)
    {
        printf("Error in binding.\n");
        exit(1);
    }
    printf("Bind to port %d\n", portNumber);

    // passive listening
    if (listen(sockfd, 10) == 0)
    {
        printf("Listening....\n");
    }
    else
    {
        printf("Error in binding.\n");
    }
    while (1)
    {

        // accept connections
        newSocket = accept(sockfd, (struct sockaddr *)&newAddr, &addr_size); // accept a new connection
        if (newSocket < 0)
        {
            exit(1);
        }

        printf(">> Connection accepted from %s:%d\n", inet_ntoa(newAddr.sin_addr), ntohs(newAddr.sin_port));
        printf(">> But Client is not yet Logged In.\n");
        if ((childpid = fork()) == 0)
        {
            close(sockfd);

            while (1)
            {
                recv(newSocket, buffer, 1024, 0); // receiving input commands from client

                // handle USER command
                if (strncmp(buffer, "USER", 4) == 0)
                {
                    logged_in = true;
                    printf(">> Client %s has Successfully Logged In.\n");
                    bzero(buffer, sizeof(buffer));
                    char msg[256] = "Reply [230]: User logged in.. please proceed";
                    send(newSocket, msg, strlen(msg), 0); // send the user logged in message to client
                }

                // handle QUIT command
                if (strncmp(buffer, "QUIT", 4) == 0)
                {
                    printf(">> Disconnected from %s:%d\n", inet_ntoa(newAddr.sin_addr), ntohs(newAddr.sin_port));
                    char mssg[256] = "Reply [221] Server disconnected.. Logging out";
                    send(newSocket, mssg, strlen(mssg), 0); // send message to client
                    break;
                }

                if (logged_in == true)
                {
                    // handle change directory logic
                    if (strncmp(buffer, "CWD", 3) == 0)
                    {
                        // printf("\nInside CWD logic");
                        int k = 0;
                        char ans[1024];
                        for (int i = 4; i < strlen(buffer); i++)
                        {
                            ans[k] = buffer[i];
                            k++;
                        }
                        strtok(ans, "\n");                 // get the argument passed
                        int status = changeDirectory(ans); // change the directory based on argument passed
                        if (status == 0)
                        {
                            // show the current directory
                            status = show_currentDirectory();
                            if (status == 343)
                            {
                                // if current directory status returned successfully
                                static char buf[256];
                                char mssg[256] = "Reply[200]: Command is Okay.\nReply[343]: Remote Working Directory: ";
                                strcat(mssg, pwdbuf);
                                send(newSocket, mssg, strlen(mssg), 0);
                            }
                        }
                    }
                    // handle CDUP directory
                    else if (strncmp(buffer, "CDUP", 4) == 0)
                    {
                        // printf("\nInside CDUP logic");
                        char *ans = "..";
                        int status = changeDirectory(ans); // change the dorectory based on argument passed
                        if (status == 0)
                        {
                            // show the current directory
                            status = show_currentDirectory();
                            if (status == 343)
                            {
                                // if current directory status returned successfully
                                static char buf[256];
                                char mssg[256] = "Reply[200]: Command is Okay.\nReply[343]: Remote Working Directory: ";
                                strcat(mssg, pwdbuf);
                                send(newSocket, mssg, strlen(mssg), 0);
                            }
                        }
                    }
                    // display the file titles in the directory
                    else if (strncmp(buffer, "LIST", 4) == 0)
                    {
                        // printf("inside the list func");
                        int status = ListFilesInDirectory();
                        if (status == 0)
                        {
                            strcpy(buflist, list);
                            send(newSocket, buflist, strlen(buflist), 0); // send the listed files to client side
                        }
                        else if (status == 336)
                        {
                            char statmsg[256];
                            strcpy(statmsg, "Reply [346] Error in opening the directory");
                            send(newSocket, statmsg, strlen(statmsg), 0); // send the error msg to client
                        }
                    }
                    // handle Make directory
                    else if (strncmp(buffer, "MKD", 3) == 0)
                    {
                        // printf("Inside MKD func");
                        int k = 0;
                        char ans[1024];
                        for (int i = 4; i < strlen(buffer); i++)
                        {
                            ans[k] = buffer[i];
                            // printf("Each character: %c\n",ans[k]);
                            k++;
                        }
                        strtok(ans, "\n");               // get the arguments
                        int status = makeDirectory(ans); // passing name of the folder to be created as an argument
                        if (status == 336)
                        {
                            // successfully created
                            char mssg[256] = "Reply [257] Directory Successfully created ";
                            strcat(mssg, ans);
                            send(newSocket, mssg, strlen(mssg), 0); // send the new directory created message to client
                            bzero(mssg, strlen(mssg));
                        }
                        else if (status == 337)
                        {
                            // already exists
                            char mssg[256] = "Reply [337] Directory Already Exists, so cannot be created again ";
                            send(newSocket, mssg, strlen(mssg), 0);
                        }
                    }
                    // handle RMD function
                    else if (strncmp(buffer, "RMD", 3) == 0)
                    {
                        // printf("inside RMD func");
                        int k = 0;
                        char ans[1024];
                        for (int i = 4; i < strlen(buffer); i++)
                        {
                            ans[k] = buffer[i];
                            // printf("Each character: %c\n",ans[k]);
                            k++;
                        }
                        strtok(ans, "\n");       // getting the argument value
                        int status = rmdir(ans); // removing the directory passed as an argument
                        if (status == 0)
                        {
                            // successfully deleted
                            char mssg[256] = "Reply [200] Command okay..  Reply [344] Directory Successfully deleted ";
                            send(newSocket, mssg, strlen(mssg), 0);
                        }
                        else if (status == -1)
                        {
                            // not deleted somehow
                            char mssg[256] = "Reply [200] Command okay.. Reply [345] Not successful in deleting. Directory does not exist ";
                            send(newSocket, mssg, strlen(mssg), 0);
                        }
                    }
                    // handle present working directory logic
                    else if (strncmp(buffer, "PWD", 3) == 0)
                    {
                        // printf("\n Inside PWD logic\n");
                        int status = show_currentDirectory(); // display the current directory
                        if (status == 343)
                        {
                            // if current directory status returned successfully
                            // static char buf[256];
                            char mssg[256] = "Reply[200]: Command is Okay.\nReply[343]: Remote Working Directory: ";
                            // split(pwdbuf);
                            strcat(mssg, pwdbuf);
                            send(newSocket, mssg, strlen(mssg), 0);
                        }
                    }
                    // handle STOR command
                    else if (strncmp(buffer, "STOR ", 5) == 0)
                    {
                        // printf("Display the stor byffer value: %s", buffer);
                        // printf("Inside the STOR func");
                        int k = 0;
                        char ans[1024];
                        for (int i = 5; i < strlen(buffer); i++)
                        {
                            ans[k] = buffer[i];
                            // printf("Each character: %c\n",ans[k]);
                            k++;
                        }
                        strtok(ans, "\n"); // get the first argument
                        // printf("\nfilename to be stored from client to server: %s\n", ans);
                        char *answer = ans;
                        // sleep(2);
                        send(newSocket, answer, strlen(answer), 0); // send the filename to client
                        // sleep(2);

                        int status = write_file(answer, newSocket); // passing the filename as an argument
                        // if write file was successful, send message to client
                        if (status == 0)
                        {
                            char mssg[256] = "Reply [200].. Command okay. File uploaded in Server sucessfully";
                            send(newSocket, mssg, strlen(mssg), 0);
                        }
                        else if (status == -1)
                        {
                            char mssg[256] = "Some Error happened in receiving socket";
                            send(newSocket, mssg, strlen(mssg), 0);
                        }
                    }
                    // handle RETR logic
                    else if (strncmp(buffer, "RETR ", 5) == 0)
                    {
                        // printf("Inside the RETR func");
                        int k = 0;
                        char ans[1024];
                        char temp[1024];
                        for (int i = 5; i < strlen(buffer); i++)
                        {
                            ans[k] = buffer[i];
                            // printf("Each character: %c\n",ans[k]);
                            k++;
                        }
                        strtok(ans, "\n");
                        // printf("\nfilename to be downloaded to client from server: %s\n", ans);
                        strcpy(temp, ans);

                        // get file contents from server
                        int status = retr_file(ans); // passing the filename as an argument
                        if (status == 0)
                        {
                            send(newSocket, temp, MAXLINE, 0); // send the message to client
                            // printf("\nWhat are the file contents: %s\n", file);
                            send(newSocket, file, MAXLINE, 0); // send the file contents
                            printf(">> File Sent Successfully.\n");
                        }
                        else
                        {
                            // handle the error portions..
                            char statmsg[1024];
                            if (status == 347)
                            {
                                strcpy(statmsg, "Reply [347] Invalid Filename.. File does not exist..");
                                send(newSocket, statmsg, strlen(statmsg), 0);
                            }
                            else if (status == 348)
                            {
                                strcpy(statmsg, "Reply [348] Error opening file..");
                                send(newSocket, statmsg, strlen(statmsg), 0);
                            }
                            // printf("Some error");
                        }
                    }
                    // append logic will append file or create new one if doesnot exist
                    else if (strncmp(buffer, "APPE ", 5) == 0)
                    {
                        int k = 0;
                        int l = 0;
                        // char ans[1024];
                        // char content[1024];
                        char temp[1024];
                        char *token;
                        strcpy(temp, buffer);
                        // printf("\n What's the contents: %s\n", temp);
                        bzero(buffer, sizeof(buffer));

                        // get the command
                        token = strtok(temp, " ");
                        // printf("token word: %s\n", token);

                        int count_token = 0;
                        char *token_word[256];
                        while (token != NULL)
                        {
                            // printf(" token: %s\n", token);
                            token_word[count_token] = token;
                            // printf("Token word in array is: %s\n", token_word[count_token]);
                            count_token += 1;
                            token = strtok(NULL, " ");
                        }

                        // printf("LEngth of ans before : %d\n", strlen(token_word[1]));
                        // printf("Length of content before : %d\n", strlen(token_word[2]));
                        strtok((token_word[1]), " ");
                        strtok(token_word[2], "\n");
                        // printf("LEngth of ans after : %d\n", strlen(token_word[1]));
                        // printf("Length of content after : %d\n", strlen(token_word[2]));
                        // printf("\nfile to be appended to client from server:%s\n", token_word[1]);
                        // printf("\nfile to be appended to client from server:%s\n", token_word[2]);

                        int status = appe_file(token_word[1], token_word[2]);
                        if (status == 0)
                        {
                            printf(">> File appended Successfully.\n");
                            strcpy(buffer, "\nReply [200].. File appended successfully\n");
                            send(newSocket, buffer, MAXLINE, 0);
                        }
                        else
                        {
                            printf("Some error");
                        }
                    }
                    // give OK respnse to server
                    else if (strncmp(buffer, "NOOP", 4) == 0)
                    {
                        strcpy(buffer, "Reply [200].. Command OK. Server running fine..\n");
                        send(newSocket, buffer, strlen(buffer), 0);
                    }
                    // delete file from server
                    else if (strncmp(buffer, "DELE ", 5) == 0)
                    {
                        int k = 0;
                        char ans[1024];
                        // char temp[1024];

                        for (int i = 5; i < strlen(buffer); i++)
                        {
                            ans[k] = buffer[i];
                            // printf("Each character: %c\n",ans[k]);
                            k++;
                        }
                        strtok(ans, "\n"); // get the argument
                        // printf("\nfilename is %s\n", ans);
                        int status = remove(ans); // delete file
                        if (status == 0)
                        {
                            strcpy(buffer, "Reply [200] File deleted Successfully\n");
                            send(newSocket, buffer, strlen(buffer), 0);
                        }
                        else if (status == -1)
                        {
                            strcpy(buffer, "Reply [550] File does not exist. Cannot be deleted..\n");
                            send(newSocket, buffer, strlen(buffer), 0);
                        }
                    }
                    // handle ABOR
                    else if (strncmp(buffer, "ABOR", 4) == 0)
                    {
                        int status = abort_service();
                        if (status == 226 || status == 446)
                        {
                            // logged_in = false;
                            strcpy(buffer, "Reply [226] Service has been aborted !!\n");
                            send(newSocket, buffer, strlen(buffer), 0);
                        }
                        else
                        {
                            strcpy(buffer, "Reply [400] Command Failed.. No Process running..!!\n");
                            send(newSocket, buffer, strlen(buffer), 0);
                        }
                    }
                    // handle REIN
                    else if (strncmp(buffer, "REIN", 4) == 0)
                    {
                        printf("reinitialized\n");
                        strcpy(buffer, "Reply [200].. Command okay.. reinitializing server\n");
                        send(newSocket, buffer, strlen(buffer), 0);
                        logged_in = false;                                                                    // setting the logged_in flag to true
                        strcpy(buffer, "Reply [221] user logged out successfully!!, Please login again!!\n"); // in future we will be using boolean flag to set the login value to true and false
                        send(newSocket, buffer, strlen(buffer), 0);
                    }
                    // handle RNFR and RNTO
                    else if (strncmp(buffer, "RNFR ", 5) == 0)
                    {

                        // printf("Display the stor buffer value: %s", buffer);

                        int k = 0;
                        char ans[1024];
                        char temp[1024];
                        strcpy(temp, buffer);
                        bzero(buffer, sizeof(buffer));
                        for (int i = 5; i < strlen(temp); i++)
                        {
                            ans[k] = temp[i];
                            // printf("Each character: %c\n",ans[k]);
                            k++;
                        }
                        strtok(ans, "\n");
                        // printf("\nfilename is %s\n", ans);
                        strcpy(renfr, ans);
                        strcpy(buffer, " Reply [200] Commands Ok.. You need to enter RNTO command now!!\n");
                        send(newSocket, buffer, strlen(buffer), 0);
                    }
                    else if (strncmp(buffer, "RNTO ", 5) == 0)
                    {

                        // printf("Display the stor buffer value: %s", buffer);

                        int k = 0;
                        char ans[1024];
                        char temp[1024];
                        strcpy(temp, buffer);
                        bzero(buffer, sizeof(buffer));
                        for (int i = 5; i < strlen(temp); i++)
                        {
                            ans[k] = temp[i];
                            // printf("Each character: %c\n",ans[k]);
                            k++;
                        }
                        strtok(ans, "\n");
                        // printf("\nfilename is %s\n", ans);
                        rename(renfr, ans); // rename the file
                        strcpy(buffer, " Reply [200] Filename is changed successfully!!\n");
                        send(newSocket, buffer, strlen(buffer), 0);
                    }
                    else if (strncmp(buffer, "STAT", 4) == 0 || strncmp(buffer, "REST", 4) == 0)
                    {
                        char temp[1024];
                        strcpy(temp, "Reply [502] Command not implemented\n");
                        send(newSocket, temp, strlen(temp), 0);
                    }
                    else
                    {
                        // printf("Client: %s\n", buffer);
                        send(newSocket, buffer, strlen(buffer), 0); // send the buffer message to client
                        bzero(buffer, sizeof(buffer));
                    }
                }
                else
                {
                    bzero(buffer, sizeof(buffer));
                    char mssg[256] = "\nReply[530]: not logged in..\n";
                    send(newSocket, mssg, strlen(mssg), 0);
                }
                bzero(buffer, sizeof(buffer)); // clearing out buffer
            }
            bzero(buffer, sizeof(buffer)); // clearing out buffer
        }
    }
    close(newSocket); // close the socket connection
    return 0;
}

// write the contents in the file
int write_file(char *filename, int sockfd)
{
    int n;
    FILE *fp;
    char buffer[MAXLINE];
    char clientContent[MAXLINE];
    // printf("file name %s\n", filename);
    fp = fopen(filename, "w");
    // printf("hello there\n");
    if (fp == NULL)
    {
        perror("[-]Error in creating file.");
        exit(1);
    }
    while (1)
    {
        n = recv(sockfd, buffer, MAXLINE, 0); // receive contents from client via socket connection
        if (n <= 0)
        {
            break;
            return -1;
        }
        strcpy(clientContent, buffer);
        bzero(buffer, sizeof(buffer));
        // printf("\nDisplay the clientContent: %s\n", clientContent);
        fprintf(fp, "%s", clientContent);
        fclose(fp);
        // sleep(1);

        printf("\n>> File Name: %s", filename);
        printf("\n>> File Size: %ld bytes\n", strlen(clientContent));
        // close(sockfd);
        return 0;
    }
    return 0;
}

// function to switch to another directory
int changeDirectory(char *directory)
{
    strtok(directory, "\n");
    if (chdir(directory) == -1)
    {
        if (errno == EACCES)
            return 339;
        else if (errno == ENOTDIR || errno == ENOENT)
            return 340;

        else
            return 341;
    }
    return 0; // if successful
}

// function to show the current directory
int show_currentDirectory()
{
    // char pwdbuf[256];
    // printf("\ninside cirrent direcl logic");

    // display logic for the current directory
    if (getcwd(pwdbuf, 256) == NULL)
    {
        printf("Some issue");
        return 342;
    }

    strtok(pwdbuf, "\n");
    //printf("\nCurrent Directory >> %s", pwdbuf);
    return 343;
}

// logic to handle the listing of files in a directory
int ListFilesInDirectory()
{

    DIR *dp;
    struct dirent *DIRP;
    *list = '\0';

    if ((dp = opendir("./")) == NULL)
    {
        return 346;
    }

    strcat(list, "Reply [200] Command Okay.. Displaying contents of the directory below..\n");
    while ((DIRP = readdir(dp)) != NULL)
    {
        if ((strcmp(DIRP->d_name, ".") != 0) && (strcmp(DIRP->d_name, "..") != 0))
        {
            strcat(list, "--> ");
            strcat(list, DIRP->d_name);
            strcat(list, "\n");
        }
    }

    closedir(dp);

    return 0;
}

// handing logic to make directory
int makeDirectory(char *directory)
{

    strtok(directory, "\n"); // to remove trailing '\n'
    int status = stat(directory, &st);

    if (status == -1)
    {
        mkdir(directory, 0700); // make directory logic with create mode
        return 336;             // when a new directory created
    }

    if (status == 0)
        return 337; // if already directory created

    return 338;
}

// retrieve the file contents from server to client
int retr_file(char *filename)
{

    strtok(filename, "\n"); // to remove trailing '\n'

    char ch;
    FILE *f;

    if ((f = fopen(filename, "r")) == NULL)
    {
        if (errno == ENOENT)
        {
            return 347; // Invalid filename. File does not exist
        }
        return 348; // Error opening file
    }

    int k = 0;
    ch = fgetc(f);
    while (ch != EOF)
    {
        file[k] = ch;
        // sleep(2);
        k++;
        ch = fgetc(f);
    }
    file[k] = '\0';

    return 0;
}

// function to append contents from one file to another file
int appe_file(char *filename, char *filename2)
{
    // declaring file pointers
    FILE *fp, *fp2;
    char temp[256];

    fp = fopen(filename, "r");   // opening the file in read mode
    fp2 = fopen(filename2, "a"); // opening another file in append mode
    if (fp == NULL)
    {
        perror("Error opening file.");
        return 1;
    }
    else
    {
        while (fgets(temp, sizeof(temp), fp))
        {
            fprintf(fp2, "%s", temp); // write the contents in the file
        }
    }
    fclose(fp);
    fclose(fp2);
    return 0;
}

// function to abort service
int abort_service()
{

    srand(time(NULL));
    int no = (rand() % 2);
    printf(">> Client is abnormally logging out.\n");
    if (no == 0)
        return 226; // return 226: abort command is successfully processed.
    return 426;     // return 426: Request Action Aborted. Data service aborted abnormally.
}
