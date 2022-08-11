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

// #define PORT 10075
#define MAXLINE 1024
int newSocket;
int count = 0;

int main(int argc, char *argv[]) {

    /*
        Objective:      To create a socket between FTP-CLIENT and FTP-SERVER & accepting
                        client commands using recv-line and sending response to client
                        using send-line.

    */
    if(argc != 2){
      printf("error");
    }
    printf("\e]2;FTP-Server\a");
    // system("clear");

    int ret, portNumber;
    struct sockaddr_in serverAddr;      // Server Socket Address Structure
    struct sockaddr_in newAddr;         // Client Socket Address Structure
    socklen_t addr_size;                // Value Result Argument.

    int sockfd;
    char buffer[MAXLINE];
    pid_t childpid;

    /*================================================SOCKET-CREATION================================================*/
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if(sockfd < 0){
        printf("Error in connection.\n");
        exit(1);
    }


    memset(&serverAddr, '\0', sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;
    sscanf(argv[1], "%d", &portNumber);                        // IPv4
    serverAddr.sin_port = htons((uint16_t)portNumber);
    serverAddr.sin_addr.s_addr = inet_addr("127.0.0.1");        //Server IP-address of '127.0.0.1'
    printf("Server Socket is created with IP-address %s.\n", inet_ntoa(serverAddr.sin_addr));

    /*================================================REUSE-SAME-PORT================================================*/
    // int optval = 1;
    // setsockopt(sockfd, SOL_SOCKET, SO_REUSEPORT, &optval, sizeof(optval));  // To reuse same Port multiple times

    /*================================================PERFORM-BINGING================================================*/
    ret = bind(sockfd, (struct sockaddr*)&serverAddr, sizeof(serverAddr));

    if(ret < 0){
        printf("Error in binding.\n");
        exit(1);
    }
    printf("Bind to port %d\n", portNumber);

    /*===============================================PASSIVE-LISTENING===============================================*/
    if(listen(sockfd, 10) == 0) {
        printf("Listening....\n");
    }
    else {
        printf("Error in binding.\n");
    }
    while(1){

        /*================================================ACCEPT-CONNECTIONS================================================*/
        newSocket = accept(sockfd, (struct sockaddr*)&newAddr, &addr_size);     // accept a new connection
        if(newSocket < 0){
            exit(1);
        }

        printf(">> Connection accepted from %s:%d\n", inet_ntoa(newAddr.sin_addr), ntohs(newAddr.sin_port));
        printf(">> But Client is not yet Logged In.\n");
        if((childpid = fork()) == 0){
			       close(sockfd);

			while(1){
				recv(newSocket, buffer, 1024, 0);
				if(strcmp(buffer, "quit") == 0){
					printf("Disconnected from %s:%d\n", inet_ntoa(newAddr.sin_addr), ntohs(newAddr.sin_port));
					break;
				}else{
					printf("Client: %s\n", buffer);
					send(newSocket, buffer, strlen(buffer), 0);
					bzero(buffer, sizeof(buffer));
				}
			}
		}
  }
      close(newSocket);
    return 0;
}
