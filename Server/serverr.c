#include "server.h"

int newSocket;
int count = 0;
char file_name[100]; // to store contents from client in server
char pwdbuf[256];
char list[1024];
struct stat st = {0};

void split(char *pathaname);
//int store_file(char *pathname);
int changeDirectory(char *directory);
int show_currentDirectory();
int ListFilesInDirectory();
int makeDirectory(char *directory);

int main(int argc, char *argv[])
{
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
    char buflist[MAXLINE];
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
                bzero(buffer, sizeof(buffer));
                recv(newSocket, buffer, 1024, 0); // receiving input commands from client

                // handle QUIT
                if (strcmp(buffer, "QUIT") == 0)
                {
                    printf("Disconnected from %s:%d\n", inet_ntoa(newAddr.sin_addr), ntohs(newAddr.sin_port));
                    break;
                }

                // // handle STOR command
                // else if (strncmp(buffer, "STOR ", 5) == 0)
                // {
                //     // STOR command received from client end
                //     printf("Display the stor byffer value: %s", buffer);
                //     int k = 0;
                //     char ans[1024];
                //     for (int i = 5; i < strlen(buffer); i++)
                //     {
                //         ans[k] = buffer[i];
                //         // printf("Each character: %c\n",ans[k]);
                //         k++;
                //     }
                //     strtok(ans, "\n");
                //     printf("\nfilename is %s\n", ans);
                //     int status = store_file(ans); // passing the filename as an argument
                // }

                // handle change directory logic
                else if (strncmp(buffer, "CWD", 3) == 0)
                {
                    printf("\nInside CWD logic");
                    int k = 0;
                    char ans[1024];
                    for (int i = 4; i < strlen(buffer); i++)
                    {
                        ans[k] = buffer[i];
                        // printf("Each character: %c\n",ans[k]);
                        k++;
                    }
                    strtok(ans, "\n");
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
                            // split(pwdbuf);
                            strcat(mssg, pwdbuf);
                            send(newSocket, mssg, strlen(mssg), 0);
                            // strcpy(buf, mssg);
                            // strcat(buf, "\nCurrent-Working-Directory-Name is : ");
                            // strcat(buf, file_name);
                            // return buf;
                        }
                    }
                }
                // handle CDUP directory
                else if (strncmp(buffer, "CDUP", 4) == 0)
                {
                    printf("\nInside CDUP logic");
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
                            // split(pwdbuf);
                            strcat(mssg, pwdbuf);
                            send(newSocket, mssg, strlen(mssg), 0);
                            // strcpy(buf, mssg);
                            // strcat(buf, "\nCurrent-Working-Directory-Name is : ");
                            // strcat(buf, file_name);
                            // return buf;
                        }
                    }
                }
                // handle list
                else if (strncmp(buffer, "LIST", 4) == 0)
                {
                    printf("inside the list func");
                    int status = ListFilesInDirectory();
                    if (status == 0)
                    {
                        strcpy(buflist,list);
                        send(newSocket,buflist,strlen(buflist),0); // send the listed files to client side
                    }

                }
                // handle MKD
                else if (strncmp(buffer,"MKD",3) == 0)
                {
                    printf("Inside MKD func");
                    int k = 0;
                    char ans[1024];
                    for (int i = 4; i < strlen(buffer); i++)
                    {
                        ans[k] = buffer[i];
                        // printf("Each character: %c\n",ans[k]);
                        k++;
                    }
                    strtok(ans, "\n");
                    int status = makeDirectory(ans); // passing name of the folder to be created as an argument
                    if (status == 336){
                        // successfully created
                        char mssg[256] = "Directory Successfully created --> ";
                        strcat(mssg,ans);
                        send(newSocket,mssg,strlen(mssg),0); // send the new directory created message to client
                        bzero(mssg,strlen(mssg));
                    }
                    else if (status == 337){
                        // already exists
                        char mssg[256] = "Directory Already Exists, so cannot be created again ";
                        send(newSocket,mssg,strlen(mssg),0);
                    }

                } // handle RMD function
                else if (strncmp(buffer,"RMD",3) == 0)
                {
                    printf("inside RMD func");
                    int k = 0;
                    char ans[1024];
                    for (int i = 4; i < strlen(buffer); i++)
                    {
                        ans[k] = buffer[i];
                        // printf("Each character: %c\n",ans[k]);
                        k++;
                    }
                    strtok(ans, "\n");
                    int status = rmdir(ans);
                    if (status == 0){
                        // successfully deleted
                        char mssg[256] = "Directory Successfully deleted ";
                        send(newSocket,mssg,strlen(mssg),0);
                    }
                    else if (status == 1){
                        // not delted somehow
                        char mssg[256] = "Not successful in deleting ";
                        send(newSocket,mssg,strlen(mssg),0);
                    }

                }
                // handle present working directory logic
                else if (strncmp(buffer, "PWD ", 4) == 0 || strncmp(buffer, "pwd ", 4) == 0 ||
                         strncmp(buffer, "PWD\t", 4) == 0 || strncmp(buffer, "pwd\t", 4) == 0 ||
                         strncmp(buffer, "PWD\n", 4) == 0 || strncmp(buffer, "pwd\n", 4) == 0)
                {
                    printf("\n Inside PWD logic");
                    int status = show_currentDirectory();
                    if (status == 343)
                        {
                            // if current directory status returned successfully
                            static char buf[256];
                            char mssg[256] = "Reply[200]: Command is Okay.\nReply[343]: Remote Working Directory: ";
                            // split(pwdbuf);
                            strcat(mssg, pwdbuf);
                            send(newSocket, mssg, strlen(mssg), 0);
                            // strcpy(buf, mssg);
                            // strcat(buf, "\nCurrent-Working-Directory-Name is : ");
                            // strcat(buf, file_name);
                            // return buf;
                        }
                }

                else
                {
                    // printf("Client: %s\n", buffer);
                    send(newSocket, buffer, strlen(buffer), 0);
                    bzero(buffer, sizeof(buffer));
                }
                bzero(buffer, sizeof(buffer));
            }
        }
    }
    close(newSocket);
    return 0;
}

// void split(char *pathname)
// {

//     /*
//         Objective:      To extract the filename from the pathname
//         Return Type:    void(null)
//         Parameter:
//             char *pathname: specifes pathname from which filename to be extracted.
//         Approach:       perform using string traversal and delimiting it by '/'
//     */

//     char splitStrings[10][10];
//     int i, j, cnt;

//     j = 0;
//     cnt = 0;
//     for (i = 0; i <= (strlen(pathname)); i++)
//     {
//         if (pathname[i] == '/' || pathname[i] == '\0')
//         {
//             splitStrings[cnt][j] = '\0';
//             cnt++; // for next word
//             j = 0; // for next word, init index to 0
//         }
//         else
//         {
//             splitStrings[cnt][j] = pathname[i];
//             j++;
//         }
//     }
//     strcpy(file_name, splitStrings[cnt - 1]);
//     file_name[strlen(file_name)] = '\0';
// }

// int store_file(char *pathname)
// {

//     /*
//         Objective:      To store the file sent by client at the server side.
//         Return Type:    Integer
//                             return 0:   succesfully stored file at server side.
//         Parameter:
//             char *pathname: specifies the path of the file present at client side
//                             i.e. to be stored at server side.
//         Approach:       performed by opening the file at client side and storing it
//                         at the server side(server Directory) by copying the contents.
//     */
//     printf("\n PAthname iss %s", pathname);
//     printf("\nInide the storefile func\n");
//     char buf[1024];
//     char res[1024];
//     printf("\n PAthname is %s", pathname);

//     int socketstatus = recv(newSocket, buf, strlen(buf), 0);
//     printf("\nSocket Status %d", socketstatus);
//     if (recv(newSocket, buf, strlen(buf), 0) < 0)
//     {
//         // if not receiving
//         printf("\n socket issue");
//     }
//     printf("\n Socket buffer Vakue is %s\n", buf);

//     if (strncmp(buf, "NULL", 4) == 0)
//     {
//         printf(">> Invalid File-Name to Store.\n");
//         return -1;
//     }

//     strcpy(res, buf);
//     bzero(buf, sizeof(buf));

//     printf("\nPathanme before splitting %s", pathname);
//     split(pathname);
//     strtok(pathname, "\n");
//     char dest[256];
//     getcwd(dest, 256);
//     strcat(dest, "/");
//     strtok(file_name, "\n");
//     strcat(dest, file_name);
//     dest[strlen(dest)] = '\0';

//     FILE *fp1;
//     fp1 = fopen(dest, "w");
//     fprintf(fp1, "%s", res);
//     fclose(fp1);

//     printf("\n-----------------------------------------------------------------------");
//     // printf("\n>> File Received Successfully by client - %s.", USERNAME[count]);
//     printf("\n>> File Saved in Path %s", dest);
//     printf("\n>> File Name: %s", file_name);
//     printf("\n>> File Size: %ld bytes\n", strlen(res));
//     printf("-----------------------------------------------------------------------\n");

//     return 0;
// }

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

int show_currentDirectory()
{
    //char pwdbuf[256];
    printf("\ninside cirrent direcl logic");
    // display logic for the current directory
    if (getcwd(pwdbuf, 256) == NULL)
    {
        printf("Some issue");
        return 342;
    }
        
    strtok(pwdbuf, "\n");
    printf("\nCurrent Directory >> %s", pwdbuf);
    return 343;
}

// logic to handle the listing of files in a directory
int ListFilesInDirectory() 
{

        DIR *dp;
        struct dirent *DIRP;
        *list = '\0';

        // if (strlen(arg) > 1) {
        //     strtok(arg, "\n");
        //     if ((directory = opendir(arg)) == NULL)
        //         return 346;
        // }
        // else {
        //     if((directory = opendir("./")) == NULL) {
        //         return 346;
        //     }
        // }
        if((dp = opendir("./")) == NULL) {
                return 346;
            }
	
	    while((DIRP = readdir(dp)) != NULL) {
            if((strcmp(DIRP->d_name, ".") != 0) && (strcmp(DIRP->d_name, "..") != 0)) {
                strcat(list, "--> ");
                strcat(list, DIRP->d_name);
                strcat(list,"\n");
            
            }
        }

        closedir(dp);
    
    return 0;
}

 // handing logic to make directory
int makeDirectory(char *directory) {

    /*
        Objective:      To make a directory when Ftp-client requests 
                                MKD <path-to-new-directory>
        Return Type:    Integer
                            return 336: Directory Created
                            return 337: Directory already exists
                            return 338: Directory can't be created
        Parameter:      
            char *directory: path of directory to be created
        Approach:       checking existence of directory using 'stat' function
                        and creating directory using 'mkdir' 

    */

    strtok(directory, "\n");    // to remove trailing '\n'
    int status = stat(directory, &st);
    
    if (status == -1) {
        mkdir(directory, 0700);
        return 336;
    }

    if (status == 0) 
        return 337;

    return 338;
}
