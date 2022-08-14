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

char buf[MAXLINE];
char file_name[1024];
int clientSocket;
char buffer[1024];
char uploadfile[1024];
bool logged_in = false;

// function to store the contents in a file
void store_file(char *dest, char file[1024])
{

    FILE *fp1;
    fp1 = fopen(dest, "w");
    fprintf(fp1, "%s", file);
    fclose(fp1);
}
// function to download contents from server side
void retr_result(char *filename)
{
    /*Store File at client-site.*/
    char res[1024];
    // bzero(buffer, sizeof(buffer));

    recv(clientSocket, buffer, MAXLINE, 0);
    printf("\nContent received from server file: %s\n", buffer);

    strcpy(res, buffer);
    bzero(buffer, sizeof(buffer));

    getcwd(buf, 256);
    strcat(buf, "/");
    strcat(buf, filename);
    // bzero(buffer, sizeof(buffer));
    store_file(buf, res);

    printf("File-Name: %s.\n", filename);
    printf("File-Size: %ld bytes.\n", strlen(res));
    printf("Received Successfully.\n");
    printf("FILE OK...Download Completed.");
    printf("\n");
}

// duplicate method to check (upload file contents)
int upload_file(char *fileName)
{
    strtok(fileName, "\n");

    char ch;
    FILE *f;

    if ((f = fopen(fileName, "r")) == NULL)
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
        uploadfile[k] = ch;
        k++;
        ch = fgetc(f);
    }
    uploadfile[k] = '\0';

    if (send(clientSocket, uploadfile, sizeof(uploadfile), 0) == -1)
    {
        perror("[-] Error in sending data");
        exit(1);
    }
}

int main(int argc, char *argv[])
{

    // int clientSocket;
    int ret, portNumber;
    // char buffer[MAXLINE];
    char filename[20];
    FILE *fp;
    // char retr_temp[MAXLINE];
    struct sockaddr_in serverAddr; // Server Socket Address Structure
    if (argc != 2)
    {
        printf("error");
    }

    /*================================================SOCKET-CREATION================================================*/
    clientSocket = socket(AF_INET, SOCK_STREAM, 0); // create socket
    if (clientSocket < 0)
    {
        printf("Error in connection.\n");
        exit(1);
    }
    printf("\n");

    memset(&serverAddr, '\0', sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;
    sscanf(argv[1], "%d", &portNumber);
    serverAddr.sin_port = htons((uint16_t)portNumber);
    serverAddr.sin_addr.s_addr = inet_addr("127.0.0.1");

    /*==============================================SOCKET-CONNECTION===============================================*/
    ret = connect(clientSocket, (struct sockaddr *)&serverAddr, sizeof(serverAddr)); // create connection
    // connect initiates three-way handshaking

    if (ret < 0)
    {
        printf("Error in connection.\n");
        exit(1);
    }

    printf("Client Socket is created.\n");

    while (1)
    {
        printf("Client: \t");
        // scanf("%s", &buffer); // what is this?
        fgets(buffer, MAXLINE, stdin);
        // printf("\n display the bufffer: %s\n", buffer);
        send(clientSocket, buffer, strlen(buffer), 0); // pass input commands to client

        strcpy(buf, buffer);
        // printf("\n display the buf: %s\n", buf);
        buf[strlen(buf)] = '\0';
        bzero(buffer, sizeof(buffer));

        // bzero(buffer, sizeof(buffer));
        if (recv(clientSocket, buffer, MAXLINE, 0) < 0)
        {
            printf("Error in receiving Line.\n");
            exit(1);
        }

        // handle USER logic
        if (strncmp(buf, "USER", 4) == 0)
        {
            logged_in = true;
            printf("Data Returned: \t%s\n", buffer);
            bzero(buffer, sizeof(buffer));
        }

        // handle QUIT logic
        if (strncmp(buf, "QUIT", 4) == 0)
        {
            printf("Data Returned: \t%s\n", buffer);
            bzero(buffer, sizeof(buffer));
            // bzero(buffer, sizeof(buffer));
            close(clientSocket);
            // printf("[-]Disconnected from server.\n");
            exit(1);
            // break;
        }

        if (logged_in == true)
        {
            // handle PWD
            if (strncmp(buf, "PWD", 3) == 0)
            {

                printf("Data Returned: \t%s\n", buffer);
                bzero(buffer, sizeof(buffer));
            }
            else if (strncmp(buf, "CWD", 3) == 0)
            {

                printf("Data Returned: \t%s\n", buffer);
                bzero(buffer, sizeof(buffer));
                // }
            }
            else if (strncmp(buf, "CDUP", 4) == 0)
            {

                printf("Data Returned: \t%s\n", buffer);
                bzero(buffer, sizeof(buffer));
                // }
            }
            else if (strncmp(buf, "LIST", 4) == 0)
            {

                printf("Data Returned: \t%s\n", buffer);
                bzero(buffer, sizeof(buffer));
                // }
            }
            else if (strncmp(buf, "MKD", 3) == 0)
            {

                printf("Data Returned: \t%s\n", buffer);
                bzero(buffer, sizeof(buffer));
                // }
            }
            else if (strncmp(buf, "RMD", 3) == 0)
            {
                printf("Data Returned: \t%s\n", buffer);
                bzero(buffer, sizeof(buffer));
            }

            // hanlde STOR logic
            else if (strncmp(buf, "STOR ", 5) == 0)
            {
                char upload_filename[256];
                strcpy(upload_filename, buffer);
                printf("Message received from server: %s\n", upload_filename);
                bzero(buffer, sizeof(buffer));
                upload_file(upload_filename);
                // printf("[+] File data sent successfully. \n");

                if (recv(clientSocket, buffer, 1024, 0) < 0)
                {
                    printf("[-]Error in receiving data.\n");
                    exit(1);
                }
                printf("Data Returned: \t%s\n", buffer);
                // printf("Data Returned: File uploaded successfully \t%s\n", buffer);
                bzero(buffer, sizeof(buffer));
                // sleep(1);
            }
            // hanlde RETR logic
            else if (strncmp(buf, "RETR ", 5) == 0)
            {
                char download_file[256];
                strcpy(download_file, buffer);
                printf("File to be downloaded from server: %s\n", download_file);
                bzero(buffer, sizeof(buffer));
                retr_result(download_file);
            }
            // handle append logic
            else if (strncmp(buf, "APPE ", 5) == 0)
            {
                // printf("\nWhat are the contents you need to write in the file?\n");
                // fgets
                printf("sending request to append the file\n");
                printf("\nServer Response: %s", buffer);
                bzero(buffer, sizeof(buffer));
            }
            // handle NOOP logic
            else if (strncmp(buf, "NOOP", 4) == 0)
            {
                printf("server respone: %s", buffer);
                bzero(buffer, sizeof(buffer));
            }
            // handle DELE logic
            else if (strncmp(buf, "DELE", 4) == 0)
            {
                printf("server respone: %s", buffer);
                bzero(buffer, sizeof(buffer));
            }
            // handle ABOr logicx
            else if (strncmp(buf, "ABOR", 4) == 0)
            {
                printf("server response: %s", buffer);
                bzero(buffer, sizeof(buffer));
            }
            // handle REIN
            else if (strncmp(buf, "REIN", 4) == 0)
            {
                printf("waiting for server to re-initialize\n ");
                printf("server response: %s", buffer);
                bzero(buffer, sizeof(buffer));
            }
            else if (strncmp(buf, "RNFR ", 5) == 0)
            {
                printf("Sending sever the response: \n ");
                printf("server response: %s", buffer);
                bzero(buffer, sizeof(buffer));
            }
            else if (strncmp(buf, "RNTO ", 5) == 0)
            {
                printf("Renamed file will be here soon: \n ");
                printf("server response: %s", buffer);
                bzero(buffer, sizeof(buffer));
            }
        }
        else
        {
            printf("\nYou need to login before trying the command\n");
            printf("server response: %s", buffer);
            bzero(buffer, sizeof(buffer));
        }
        bzero(buffer, sizeof(buffer));
    }
    return 0;
}
