/*
    How to run Client:
        --> gcc -o testc clientt.c
        --> ./testc "PORTNUMBER" for exmaple --> ./testc 5000
*/

// importing necessary header files
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

// declaring the needed global variables for the program
char buf[MAXLINE];
char file_name[1024];
int clientSocket;
char buffer[1024];
char uploadfile[1024];
bool logged_in = false;

// function to store the contents in a file
void store_file(char *dest, char file[1024])
{

    FILE *fp1;                // declaring the file pointer
    fp1 = fopen(dest, "w");   // opening the file in write mode using file pointer
    fprintf(fp1, "%s", file); // printing the contents in file
    fclose(fp1);              // closing the file pointer
}
// function to download contents from server side
void retr_result(char *filename) // function for retrieving the result using RETR function from the server
{
    /*Store File at client-site.*/
    char res[1024];
    // bzero(buffer, sizeof(buffer));

    recv(clientSocket, buffer, MAXLINE, 0); // recieving the data from the buffer
    // printf("\nContent received from server file: %s\n", buffer); //printing the content recieved from the buffer

    strcpy(res, buffer);           // copyting buffer content in a new variable
    bzero(buffer, sizeof(buffer)); // clearing the buffer

    getcwd(buf, 256); // get current current working directory
    strcat(buf, "/"); // concating the file name
    strcat(buf, filename);
    // bzero(buffer, sizeof(buffer));
    store_file(buf, res); // calling the store_file function listed above this function

    printf("File-Name: %s.\n", filename);           // printing filename
    printf("File-Size: %ld bytes.\n", strlen(res)); // printing the filesize of the function
    printf("Received Successfully.\n");
    printf("FILE OK...Download Completed.");
    printf("\n");
}

// duplicate method to check (upload file contents)
int upload_file(char *fileName) // function for STOR method
{
    strtok(fileName, "\n"); // seperating filename  with blank spaces

    // declaring the necessary variables
    char ch;
    FILE *f;

    // opening the file in the read mode
    if ((f = fopen(fileName, "r")) == NULL)
    {
        if (errno == ENOENT)
        {
            return 347;
        }
        return 348;
    }
    // copying the contents in the upload file variable
    int k = 0;
    ch = fgetc(f);
    while (ch != EOF)
    {
        uploadfile[k] = ch;
        k++;
        ch = fgetc(f);
    }
    uploadfile[k] = '\0';
    // sending data of upload file
    if (send(clientSocket, uploadfile, sizeof(uploadfile), 0) == -1)
    {
        perror("[-] Error in sending data");
        exit(1);
    }
}

int main(int argc, char *argv[])
{

    //***********************declaring necessary variables***********************
    // int clientSocket;
    int ret, portNumber;
    // char buffer[MAXLINE];
    char filename[20];
    FILE *fp;
    // char retr_temp[MAXLINE];
    struct sockaddr_in serverAddr;
    if (argc != 2) // if port number not specified than throw an error
    {
        printf("error");
    }

    //********************creating socket in TCP stream*****************************
    clientSocket = socket(AF_INET, SOCK_STREAM, 0); // create socket
    // throw error if condition not met
    if (clientSocket < 0)
    {
        printf("Error in connection.\n");
        exit(1);
    }
    printf("\n");
    // specifying port number, ip address and TCP
    memset(&serverAddr, '\0', sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;
    sscanf(argv[1], "%d", &portNumber);
    serverAddr.sin_port = htons((uint16_t)portNumber);
    serverAddr.sin_addr.s_addr = inet_addr("127.0.0.1");

    // establishing the connection
    ret = connect(clientSocket, (struct sockaddr *)&serverAddr, sizeof(serverAddr)); // create connection

    // throws an error if connection is not proper
    if (ret < 0)
    {
        printf("Error in connection.\n");
        exit(1);
    }

    printf("Client Socket is created.\n");

    // initializing the infinite loop
    while (1)
    {
        printf("Client: \t");
        fgets(buffer, MAXLINE, stdin);                 // takes the user input and store it in bufer
        send(clientSocket, buffer, strlen(buffer), 0); // pass input commands to client
        strcpy(buf, buffer);                           // copying buffer content in the new variable for further use
        // printf("\n display the buf: %s\n", buf);
        buf[strlen(buf)] = '\0';
        bzero(buffer, sizeof(buffer)); // clearing the buffer

        // bzero(buffer, sizeof(buffer));
        // recieve from the server
        if (recv(clientSocket, buffer, MAXLINE, 0) < 0)
        {
            printf("Error in receiving Line.\n");
            exit(1);
        }

        // handle USER logic
        // USAGE : USER => after writing this USER will successfully login
        if (strncmp(buf, "USER", 4) == 0)
        {
            logged_in = true;                       // setting the flag true if the user logs in
            printf("server respone: %s\n", buffer); // returns data sent by the server
            bzero(buffer, sizeof(buffer));          // clearing buffer
        }

        // handle QUIT logic
        // USAGE : QUIT => after writing this the server will disconnect
        if (strncmp(buf, "QUIT", 4) == 0)
        {
            printf("server respone: %s\n", buffer); // return data sent by the server
            bzero(buffer, sizeof(buffer));          // clearing the buffer
            // bzero(buffer, sizeof(buffer));
            close(clientSocket); // close the client socket as we want to handle quit command
            // printf("[-]Disconnected from server.\n");
            exit(1);
            // break;
        }

        if (logged_in == true) // if the user is logged in then only user can use other commands
        {
            // handle PWD and it is handled in server
            // USAGE : PWD => will print the working directory
            if (strncmp(buf, "PWD", 3) == 0)
            {

                printf("server respone: %s\n", buffer);
                bzero(buffer, sizeof(buffer));
            }
            // handle CWD logic whose logic is written in server
            // USAGE : CWD "location" OR .. if you want to go in parent directory
            else if (strncmp(buf, "CWD", 3) == 0)
            {

                printf("server respone: %s\n", buffer);
                bzero(buffer, sizeof(buffer));
                // }
            }
            // handle CDUP
            // USAGE : CDUP => will go in the parent directory
            else if (strncmp(buf, "CDUP", 4) == 0)
            {

                printf("server respone: %s\n", buffer);
                bzero(buffer, sizeof(buffer));
                // }
            }
            // handles the LIST command
            // USAGE : LIST => will list all the contents in the directory
            else if (strncmp(buf, "LIST", 4) == 0)
            {

                printf("server respone: \n%s\n", buffer);
                bzero(buffer, sizeof(buffer));
                // }
            }
            // handles the MKD command
            // USAGE : MKD "filename" => will make the directory according to the name specifies by the user
            else if (strncmp(buf, "MKD", 3) == 0)
            {

                printf("server respone: %s\n", buffer);
                bzero(buffer, sizeof(buffer));
                // }
            }
            // handles the RMD command
            // RMD "directory you want to delete" => will delete the directory specified
            else if (strncmp(buf, "RMD", 3) == 0)
            {
                printf("server respone: %s\n", buffer);
                bzero(buffer, sizeof(buffer));
            }

            // hanlde STOR logic
            // USAGE: STOR "file you want to upload" => will upload the given file
            else if (strncmp(buf, "STOR ", 5) == 0)
            {
                char upload_filename[256];       // creating a new variable
                strcpy(upload_filename, buffer); // copying the contents in the new variable
                printf("server respone: %s\n", upload_filename);
                bzero(buffer, sizeof(buffer)); // clearing the buffer
                upload_file(upload_filename);  // passing the argument in the function
                // printf("[+] File data sent successfully. \n");

                if (recv(clientSocket, buffer, 1024, 0) < 0) // recieving contents from the server
                {
                    printf("[-]Error in receiving data.\n");
                    exit(1);
                }
                printf("server respone: %s\n", buffer); // printing data retruned by the server
                // printf("Data Returned: File uploaded successfully \t%s\n", buffer);
                bzero(buffer, sizeof(buffer)); // clearing the buffer
            }
            // hanlde RETR logic
            // USAGE: RETR "filename" => will retrieve the given file from the server
            else if (strncmp(buf, "RETR ", 5) == 0)
            {
                char download_file[256];       // creating a new variable
                strcpy(download_file, buffer); // copying buffer contents in a new variable
                // printf("File to be downloaded from server: %s\n", download_file);
                bzero(buffer, sizeof(buffer)); // clearing the buffer
                printf("Server Response: Reply [200] Command okay..\n");
                retr_result(download_file); // passing the argument in the function
            }
            // handle append logic
            // USAGE: APPE "filename1" "filename2" => will append the given files
            else if (strncmp(buf, "APPE ", 5) == 0)
            {
                // printf("\nWhat are the contents you need to write in the file?\n");
                // fgets
                printf("sending request to append the file\n");
                printf("\nServer Response: %s", buffer);
                bzero(buffer, sizeof(buffer));
            }
            // handle NOOP logic
            // USAGE: NOOP
            else if (strncmp(buf, "NOOP", 4) == 0)
            {
                printf("server respone: %s", buffer);
                bzero(buffer, sizeof(buffer));
            }
            // handle DELE logic
            // USAGE: DELE "filename" => will dele the inputed file
            else if (strncmp(buf, "DELE", 4) == 0)
            {
                printf("server respone: %s", buffer);
                bzero(buffer, sizeof(buffer));
            }
            // handle ABOR command
            // USAGE: ABOR => will abort the process
            else if (strncmp(buf, "ABOR", 4) == 0)
            {
                printf("server response: %s", buffer);
                bzero(buffer, sizeof(buffer));
            }
            // handle REIN
            // USAGE: REIN => will reinitialize the server
            else if (strncmp(buf, "REIN", 4) == 0)
            {
                printf("waiting for server to re-initialize\n ");
                printf("server response: %s", buffer);
                logged_in = false;
                bzero(buffer, sizeof(buffer));
                
            }
            // handles RNFR command
            // USAGE: RNFR "filename" => will start the rename process of the given filename the RNTO command should be followed
            else if (strncmp(buf, "RNFR ", 5) == 0)
            {
                printf("Sending sever the response: \n ");
                printf("server response: %s", buffer);
                bzero(buffer, sizeof(buffer));
            }
            // handles RNTO command
            // USAGE: RNTO "filename" => will rename the filename inputed in the RNFR
            else if (strncmp(buf, "RNTO ", 5) == 0)
            {
                printf("Renamed file will be here soon: \n ");
                printf("server response: %s", buffer);
                bzero(buffer, sizeof(buffer));
            }
            // hanlde STAT
            else if ((strncmp(buf, "STAT", 4) == 0) || (strncmp(buf, "REST", 4) == 0))
            {
                printf("server response: %s", buffer);
                bzero(buffer, sizeof(buffer));
            }
        }
        else
        {
            printf("\nYou need to login before trying the command\n"); // if the user is not logged in then USER has to login
            printf("server response: %s", buffer);
            bzero(buffer, sizeof(buffer));
        }
        bzero(buffer, sizeof(buffer)); // clearing the final buffer
    }
    return 0;
}
