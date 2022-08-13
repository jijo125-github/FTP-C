#include "server.h"

#define MAXLINE 1024

int newSocket;
int count = 0;
char arg[1024];
int command;
bool logged_in;
bool user_done;
char list[1024];
char file_name[100];
char file[1024];
int store_port[1000];
char pwdbuf[256];
int count;

int retr_file(char *filename)
{

    strtok(filename, "\n");

    char ch;
    FILE *f;

    if ((f = fopen(filename, "r")) == NULL)
    {
        if (errno == ENOENT)
        {
            return 347;
        }
        return 348;
    }

    int k = 0;
    ch = fgetc(f);
    while (ch != EOF)
    {
        file[k] = ch;
        k++;
        ch = fgetc(f);
    }
    file[k] = '\0';

    return 0;
}

int write_file(char *filename, int sockfd)
{
    int n;
    FILE *fp;
    char buffer[MAXLINE];
    char clientContent[MAXLINE];
    printf("file name %s\n", filename);
    fp = fopen(filename, "w");
    printf("hello there\n");
    if (fp == NULL)
    {
        perror("[-]Error in creating file.");
        exit(1);
    }
    while (1)
    {
        n = recv(sockfd, buffer, MAXLINE, 0);
        if (n <= 0)
        {
            break;
            return -1;
        }
        strcpy(clientContent, buffer);
        bzero(buffer, sizeof(buffer));
        printf("\nDisplay the clientContent: %s\n", clientContent);
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

int main(int argc, char *argv[])
{

    /*
        Objective:      To create a socket between FTP-CLIENT and FTP-SERVER & accepting
                        client commands using recv-line and sending response to client
                        using send-line.

    */
    if (argc != 2)
    {
        printf("error");
    }
    printf("\e]2;FTP-Server\a");
    // system("clear");

    int ret, portNumber;
    struct sockaddr_in serverAddr; // Server Socket Address Structure
    struct sockaddr_in newAddr;    // Client Socket Address Structure
    socklen_t addr_size;           // Value Result Argument.

    int sockfd;
    char buffer[MAXLINE];
    char clientContent[MAXLINE];
    pid_t childpid;

    /*================================================SOCKET-CREATION================================================*/
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0)
    {
        printf("Error in connection.\n");
        exit(1);
    }

    memset(&serverAddr, '\0', sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;
    sscanf(argv[1], "%d", &portNumber); // IPv4
    serverAddr.sin_port = htons((uint16_t)portNumber);
    serverAddr.sin_addr.s_addr = inet_addr("127.0.0.1"); // Server IP-address of '127.0.0.1'
    printf("Server Socket is created with IP-address %s.\n", inet_ntoa(serverAddr.sin_addr));

    /*================================================REUSE-SAME-PORT================================================*/
    // int optval = 1;
    // setsockopt(sockfd, SOL_SOCKET, SO_REUSEPORT, &optval, sizeof(optval));  // To reuse same Port multiple times

    /*================================================PERFORM-BINGING================================================*/
    ret = bind(sockfd, (struct sockaddr *)&serverAddr, sizeof(serverAddr));

    if (ret < 0)
    {
        printf("Error in binding.\n");
        exit(1);
    }
    printf("Bind to port %d\n", portNumber);

    /*===============================================PASSIVE-LISTENING===============================================*/
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

        /*================================================ACCEPT-CONNECTIONS================================================*/
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
                if (recv(newSocket, buffer, MAXLINE, 0) < 0)
                {
                    char temp[MAXLINE];
                    *temp = '\0';
                    strcat(temp, "Reply[225]: Data Connection Open but No Transfer in progress.");
                    send(newSocket, temp, MAXLINE, 0);
                    continue;
                }
                // if(strncmp(buffer, "QUIT",4) == 0){
                // 	printf("Disconnected from %s:%d\n", inet_ntoa(newAddr.sin_addr), ntohs(newAddr.sin_port));
                // 	break;
                //  }
                if (strncmp(buffer, "QUIT", 4) == 0)
                {
                    // printf("Disconnected from %s:%d\n", inet_ntoa(newAddr.sin_addr), ntohs(newAddr.sin_port));
                    printf("temp disconnect\n");
                    char mssg[256] = "Server disconnected";
                    send(newSocket, mssg, strlen(mssg), 0);
                    break;
                }
                else if (strncmp(buffer, "USER", 4) == 0)
                {
                    // char temp[10];
                    // send(newSocket, temp, 10, 0);
                    printf(">> Client - %s has Successfully Logged In.\n");
                    char msg[256] = "User has successfully logged in";
                    send(newSocket, msg, strlen(msg), 0); // send the user logged in message to client
                }
                // handle RETR logic
                else if (strncmp(buffer, "RETR ", 5) == 0)
                {
                    printf("Inside the RETR func");
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
                    printf("\nfilename to be downloaded to client from server: %s\n", ans);
                    strcpy(temp, ans);

                    // get file contents from server
                    int status = retr_file(ans); // passing the filename as an argument
                    if (status == 0)
                    {
                        send(newSocket, temp, MAXLINE, 0); // send the message to client
                        send(newSocket, file, MAXLINE, 0); // send the file contents
                        printf(">> File Sent Successfully.\n");
                    }
                    else
                    {
                        printf("Some error");
                    }
                }
                else if (strncmp(buffer, "STOR ", 5) == 0)
                {
                    // printf("Display the stor byffer value: %s", buffer);
                    printf("Inside the STOR func");
                    int k = 0;
                    char ans[1024];
                    for (int i = 5; i < strlen(buffer); i++)
                    {
                        ans[k] = buffer[i];
                        // printf("Each character: %c\n",ans[k]);
                        k++;
                    }
                    strtok(ans, "\n");
                    printf("\nfilename to be stored from client to server: %s\n", ans);
                    char *answer = ans;
                    // sleep(2);
                    send(newSocket, answer, strlen(answer), 0); // send the filename to client
                    // sleep(2);

                    // handle file contents from client
                    // if (recv(newSocket, clientContent, MAXLINE, 0) < 0)
                    // {
                    //     printf("[-]Error in receiving data from client.\n");
                    //     exit(1);
                    // }
                    // printf("\nDisplay the clientContent: %s\n",clientContent);

                    int status = write_file(answer, newSocket); // passing the filename as an argument
                    // if write file was successful, send message to client
                    if (status == 0)
                    {
                        char mssg[256] = "File uploaded in Server sucessfully";
                        send(newSocket, mssg, strlen(mssg), 0);
                    }
                    else if (status == -1)
                    {
                        char mssg[256] = "Some Error happened in receiving socket";
                        send(newSocket, mssg, strlen(mssg), 0);
                    }
                }
                // else if (strncmp(buffer, "RETR ", 5) == 0)
                // {

                //     printf("Display the stor byffer value: %s", buffer);

                //     int k = 0;
                //     char ans[1024];
                //     char temp[1024];

                //     for (int i = 5; i < strlen(buffer); i++)
                //     {
                //         ans[k] = buffer[i];
                //         // printf("Each character: %c\n",ans[k]);
                //         k++;
                //     }
                //     strtok(ans, "\n");
                //     printf("\nfilename is %s\n", ans);
                //     retr_file(ans); // passing the filename as an argument

                //     strcpy(temp, ans);
                //     send(newSocket, temp, MAXLINE, 0); // send the message to client
                //     send(newSocket, file, MAXLINE, 0); // send the file contents
                //     printf(">> File Sent Successfully.\n");
                // }
                else if (strncmp(buffer, "NOOP", 4) == 0)
                {
                    strcpy(buffer, "OK\n");
                    send(newSocket, buffer, strlen(buffer), 0);
                }
                else if (strncmp(buffer, "DELE ", 5) == 0)
                {
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
                    // printf("\nfilename is %s\n", ans);
                    remove(ans);
                    strcpy(buffer, "File deleted Successfully\n");
                    send(newSocket, buffer, strlen(buffer), 0);
                }
                else
                {
                    // printf("Client: %s\n", buffer);
                    send(newSocket, buffer, strlen(buffer), 0);
                    bzero(buffer, sizeof(buffer));
                }
            }
        }
    }
    close(newSocket);
    return 0;
}
