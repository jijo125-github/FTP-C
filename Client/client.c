#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>

// #define PORT 10075
#define MAXLINE 1024
// void change_to_parent()

int main(int argc, char *argv[])
{

  int clientSocket;
	int ret, portNumber;
  char buffer[MAXLINE];
	struct sockaddr_in serverAddr;		// Server Socket Address Structure
  if(argc != 2){
    printf("error");
  }

	/*================================================SOCKET-CREATION================================================*/
	clientSocket = socket(AF_INET, SOCK_STREAM, 0);		// create socket
	if(clientSocket < 0){
		printf("Error in connection.\n");
		exit(1);
	}
	printf("\n");

	memset(&serverAddr, '\0', sizeof(serverAddr));
	serverAddr.sin_family = AF_INET;
  sscanf(argv[1],"%d",&portNumber);
	serverAddr.sin_port = htons((uint16_t)portNumber);
	serverAddr.sin_addr.s_addr = inet_addr("127.0.0.1");

	/*==============================================SOCKET-CONNECTION===============================================*/
	ret = connect(clientSocket, (struct sockaddr*)&serverAddr, sizeof(serverAddr));		// create connection
	// connect initiates three-way handshaking

	if(ret < 0){
		printf("Error in connection.\n");
		exit(1);
	}

	// system("clear");
  printf("Client Socket is created.\n");
  while(1){
		printf("Client: \t");
		scanf("%s", &buffer[0]);
		send(clientSocket, buffer, strlen(buffer), 0);
    if(strcmp(buffer,"user")==0)
    {
      printf("User logged in\n");
    }
    /*if(strcmp(buffer,"cdup")==0)
    {
      change_to_parent();
    }*/
		if((strcmp(buffer,"QUIT") == 0) || (strcmp(buffer,"quit") == 0)){
			close(clientSocket);
			printf("[-]Disconnected from server.\n");
			exit(1);
		}

		if(recv(clientSocket, buffer, 1024, 0) < 0){
			printf("[-]Error in receiving data.\n");
		}else{
			printf("Server: \t%s\n", buffer);
		}
	}
  return 0;
}
