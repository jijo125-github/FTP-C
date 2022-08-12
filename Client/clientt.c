#include "client.h"

char buf[MAXLINE];
char file_name[1024];
int clientSocket;
char buffer[1024];

// void store_command();

int main(int argc, char *argv[])
{

	int clientSocket;
	int ret, portNumber;
	char buffer[MAXLINE];
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

	// system("clear");
	printf("Client Socket is created.\n");

	while (1)
	{
		printf("Client: \t");
		// scanf("%s", &buffer); // what is this?
		fgets(buffer, MAXLINE, stdin);
		// printf("\n display the bufffer: %s\n", buffer);
		send(clientSocket, buffer, strlen(buffer), 0); // pass input commands to client

		strcpy(buf, buffer);
		printf("\n display the buf: %s\n", buf);
		buf[strlen(buf)] = '\0';
		bzero(buffer, sizeof(buffer));

		// bzero(buffer, sizeof(buffer));
		if (recv(clientSocket, buffer, MAXLINE, 0) < 0)
		{
			printf("Error in receiving Line.\n");
			exit(1);
		}

		// handle USER logic
		// printf("\n display the buf2: %s\n", buf);
		if (strcmp(buf, "USER") == 0)
		{
			printf("User logged in\n");
		}

		// handle STOR logic -- upload file to server
		// if (strncmp(buf, "STOR", 4) == 0)
		// {
		// 	printf("STOR command used\n");
		// 	/*===============================================STORE-COMMAND===============================================*/

		// 	if (strncmp(buf, "STOR", 4) == 0 || strncmp(buf, "stor", 4) == 0)
		// 		store_command(); // will work only when STOR command is called.

		// 	/*===========================================================================================================*/
		// }
		if (strncmp(buf, "PWD", 3) == 0)
		{
			// receive the returned buffer status from server
			//  if (recv(clientSocket, buffer, 1024, 0) < 0)
			//  {
			//  	printf("[-]Error in receiving data.\n");
			//  }
			//  else
			//  {
			printf("Data Returned: \t%s\n", buffer);
			bzero(buffer, sizeof(buffer));
			// }
		}
		else if (strncmp(buf, "CWD", 3) == 0)
		{
			// receive the returned buffer status from server
			//  if (recv(clientSocket, buffer, 1024, 0) < 0)
			//  {
			//  	printf("[-]Error in receiving data.\n");
			//  }
			//  else
			//  {
			printf("Data Returned: \t%s\n", buffer);
			bzero(buffer, sizeof(buffer));
			// }
		}
		else if (strncmp(buf, "CDUP", 4) == 0)
		{
			// receive the returned buffer status from server
			//  if (recv(clientSocket, buffer, 1024, 0) < 0)
			//  {
			//  	printf("[-]Error in receiving data.\n");
			//  }
			//  else
			//  {
			printf("Data Returned: \t%s\n", buffer);
			bzero(buffer, sizeof(buffer));
			// }
		}
		else if (strncmp(buf, "LIST", 4) == 0)
		{
			// receive the returned buffer status from server
			//  if (recv(clientSocket, buffer, 1024, 0) < 0)
			//  {
			//  	printf("[-]Error in receiving data.\n");
			//  }
			//  else
			//  {
			printf("Data Returned: \t%s\n", buffer);
			bzero(buffer, sizeof(buffer));
			// }
		}
		else if (strncmp(buf, "MKD", 3) == 0)
		{
			// receive the returned buffer status from server
			//  if (recv(clientSocket, buffer, 1024, 0) < 0)
			//  {
			//  	printf("[-]Error in receiving data.\n");
			//  }
			//  else
			//  {
			printf("Data Returned: \t%s\n", buffer);
			bzero(buffer, sizeof(buffer));
			// }
		}



		/*if(strcmp(buffer,"cdup")==0)
		{
		  change_to_parent();
		}*/

		if (strcmp(buf, "QUIT") == 0)
		{
			close(clientSocket);
			printf("[-]Disconnected from server.\n");
			exit(1);
		}

		// handle STOR command
		// if(strcmp())

		// if(recv(clientSocket, buffer, 1024, 0) < 0){
		// 	printf("[-]Error in receiving data.\n");
		// }else{
		// 	printf("Server: \t%s\n", buffer);
		// }
	}
	return 0;
}

// void store_command()
// {

// 	char file[1024];
// 	char ch;
// 	char ans[1024];
// 	int k = 0;

// 	/*------------------------------------------------------------------------------------------------------------*/
// 	printf("\nInside store command\n");
// 	printf("What is buf value: %s\n", buf);
// 	for (int i = 5; i < strlen(buf); i++)
// 	{
// 		ans[k] = buf[i];
// 		// printf("Each character: %c\n",ans[k]);
// 		k++;
// 	}
// 	printf("\nAnswer is %s\n", ans);
// 	strtok(ans, "\n");
// 	printf("\nfilename is %s\n", ans);
// 	/*------------------------------------------------------------------------------------------------------------*/

// 	FILE *fp;
// 	if ((fp = fopen(ans, "r")) != NULL)
// 	{

// 		k = 0;
// 		ch = fgetc(fp);
// 		while (ch != EOF)
// 		{
// 			file[k] = ch;
// 			k++;
// 			ch = fgetc(fp);
// 		}
// 		file[k] = '\0';
// 		fclose(fp);

// 		printf("Source-File-Path : %s\n", ans);
// 		printf("File-Size        : %ld bytes\n", strlen(file));

// 		strcpy(buffer, file);
// 		printf("\n Now the buffer value is %s\n", buffer);
// 		send(clientSocket, buffer, MAXLINE, 0);
// 		bzero(buffer, sizeof(buffer));
// 	}

// else
// {

// 	if (errno == ENOENT)
// 	{
// 		printf("Invalid Filename: File does not exist.\n");
// 	}
// 	else
// 		printf("Error opening file.\n");
// 	send(clientSocket, "NULL", 4, 0);
// }
/*------------------------------------------------------------------------------------------------------------*/
// }
