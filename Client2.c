//Kristina Messina
//Client
//Completed 08.03.2024

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>
#include <unistd.h>

static int clientWriteFD, clientReadFD;
static pid_t clientPID;

#define MAXBUFFER 1024
#define MAXSTRING 128

void wrapPacket(int clientWriteFD, int numSystemCall, pid_t clientPID, int numParameter, int sizeParameter, char * sendData)
{
	char packetInfo[MAXBUFFER];
	sprintf(packetInfo, "%d,%d,%d,%d,%s", numSystemCall, clientPID, numParameter, sizeParameter, sendData);
	write(clientWriteFD, &packetInfo, sizeof(packetInfo));
}

int main()
{
	char *serverIn = "/tmp/S0000";

	//Define initial variables
	clientPID = getpid();

	int numSystemCall, numParameter, sizeParameter, valueParameterInt;

	char sendData[MAXBUFFER];
	char integerStringConverter[MAXBUFFER];
	char userInput[MAXSTRING];

	//Make the client's receiving fifo
	char serverOut[128];
	sprintf(serverOut, "/tmp/C%d", getpid());
	mkfifo(serverOut, 0666);


	// Open the fifos
	clientWriteFD = open(serverIn, O_WRONLY);
	if (clientWriteFD  < 0)
	{
		perror("Error : Server input fifo not detected ");
		exit(EXIT_FAILURE);
	}
	clientReadFD = open(serverOut, O_RDWR);
	if (clientReadFD  < 0)
	{
		printf("%s : %d\n", serverOut, clientReadFD);
		perror("Error : Client input fifo not detected ");
		exit(EXIT_FAILURE);
	}
	
	int userSelection, clientConnected = 0;
	while(1)
	{
		sendData[0] = '\0';
		
		printf("--------CHOOSE------------------------------->\n| 1 : Request | 2 : Exit | 3 : Shutdown |\n :::::: ");
		scanf("%d", &userSelection);

		switch(userSelection)
		{
		case 1:
			if( clientConnected != 0 )
			{
				printf("--------CHOOSE------------------------------->\n| 2 : Number | 3 : Text | 4 : Store | 5 : Recall |\n :::::: ");
				scanf("%d", &numSystemCall);

				switch(numSystemCall)
				{
				case 2:
					printf("--------ENTER NUMBER OF PARAMETERS----------->\n :::::: ");
					scanf("%d", &numParameter);

					printf("--------ENTER PARAMETER(S)------------------->\n");
					for(int i = 1; i <= numParameter; i++)
					{
						printf("  %d  : ", i);
						scanf("%d", &valueParameterInt);

						sprintf(integerStringConverter, "%d", valueParameterInt);
						strcat(sendData, integerStringConverter);
						strcat(sendData, ";");
					}

					wrapPacket(clientWriteFD, numSystemCall, clientPID, numParameter, 4, sendData);
					
					for(int i = 1; i <= numParameter; i++)
					{
						read(clientReadFD, &valueParameterInt, 4);
						printf(">  %d recieved from %s.\n", valueParameterInt, serverIn);
					}

					break;
				case 3:
					printf("--------ENTER NUMBER OF PARAMETERS----------->\n :::::: ");
					scanf("%d", &numParameter);
					
					sizeParameter = MAXBUFFER;

					printf("--------ENTER PARAMETER(S)------------------->\n");
					for(int i = 1; i <= numParameter + 1; i++)
					{
						fgets(userInput, MAXSTRING, stdin);
						
						if(i != numParameter + 1)
						{
							printf("  %d  : ", i);
						}
						
						userInput[strlen(userInput) - 1] = 0;

						strcat(sendData, userInput);
						strcat(sendData, ";");
					}

					wrapPacket(clientWriteFD, numSystemCall, clientPID, numParameter, sizeParameter, sendData);
					
					sendData[0] = '\0';
					
					for(int i = 1; i <= numParameter; i++)
					{
						read(clientReadFD, &sendData, MAXBUFFER);
						printf(">  %s recieved from %s.\n", sendData, serverIn);
					}

					break;
				case 4:
					//Store an integer value
					printf("--------ENTER AN INTEGER TO STORE------------>\n :::::: ");
					scanf("%d", &valueParameterInt);
					
					sprintf(sendData, "%d", valueParameterInt);
					wrapPacket(clientWriteFD, numSystemCall, clientPID, numParameter, sizeParameter, sendData);

					break;
				case 5:
					//Read an integer value
					wrapPacket(clientWriteFD, numSystemCall, clientPID, 0, 0, "0");
					read(clientReadFD, &valueParameterInt, 4);

					printf(">  %d recieved from %s.\n", valueParameterInt, serverIn);

					break;
				default:
					printf(">  Not a valid selection.\n");
					break;
				}
			}
			else
			{
				printf(">  Connecting %s to %s to request upon next command.\n", serverOut, serverIn);

				numSystemCall = 1;

				wrapPacket(clientWriteFD, numSystemCall, clientPID, 0, 0, serverOut);
				
				//Verify connection successful
				read(clientReadFD, &clientConnected, 4);

				printf(">  PING RETURNED : Connected %s to %s.\n", serverOut, serverIn);
			}
			break;
		case 2:
			if(clientConnected != 0)
			{
				wrapPacket(clientWriteFD, 0, clientPID, 0, 0, "0");
			}
			printf(">  Client shutdown. . .\n");
			close(clientReadFD);
			close(clientWriteFD);
			unlink(serverOut);
			return 0;
		case 3:
			if(clientConnected != 0)
			{
				wrapPacket(clientWriteFD, -1, clientPID, 0, 0, "0");
			}
			
			//Recycling variable
			read(clientReadFD, &clientConnected, 4);
			
			if(clientConnected == 1)
			{
				printf(">  Client shutdown. . .\n");
				close(clientReadFD);
				close(clientWriteFD);
				unlink(serverOut);
				return 0;
			}
			else
			{
				printf(">  ERROR : More than this client connected.\n");
			}
			
			break;
		default:
			printf(">  Not a valid selection.\n");
			break;
		}
	}
	return 1;
}
