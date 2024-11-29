//Kristina Messina
//Server
//Completed 08.03.2024

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>
#include <unistd.h>

static int serverReadFD;

#define MAXBUFFER 1024
#define MAXSTRING 128

//Make storage structure
typedef struct N
{
	pid_t clientTag;
	int writeFD;
	char FIFO[128];
	struct N * next;
} node;

void printResults(int numSystemCall, pid_t clientPID, int numParameter, int sizeParameter, char * valueParameter)
{
	printf("| syscall : %d | pid : %d | numparam : %d | sizeparam : %d | valparam : %s |\n", numSystemCall, clientPID, numParameter, sizeParameter, valueParameter);
}

int main()
{
	//Keep track of the number of clients
	int numConnectedClients = 0;
	
	
	//Define head node and make node assist structures
	node * head = NULL;
	head = (node *) malloc(sizeof(node));
	if (head == NULL) {
		perror(">  ERROR : Nodes not initialized.\n");
		exit(EXIT_FAILURE);
	}
	
	node * tail = head, * search = head, * bridge = head;
	
	

	
	//Good to store all required characters needed in single location
	char packetInfo[MAXBUFFER];

	char *serverIn = "/tmp/S0000";

	unlink(serverIn);

	mkfifo (serverIn, 0666);

	//Open the server reading fifo
	serverReadFD = open(serverIn, O_RDWR);
	if (serverReadFD < 0)
	{
		printf("%s : %d\n", serverIn, serverReadFD);
		perror(">  ERROR : Server input fifo not detected.\n");
		exit(EXIT_FAILURE);
	}

	//Define initial variables
	int numSystemCall = 0, numParameter, sizeParameter, storedValue;
	char * valueParameter;

	pid_t clientPID;


	int serverWriteFD;


	int integerStringConverter = 0;
	while(1)
	{
		printf("--------WAITING ON CLIENT-------------------->\n");
		
		read(serverReadFD, &packetInfo, MAXBUFFER);

		numSystemCall = atoi(strtok(packetInfo, ","));
		clientPID = atoi(strtok(NULL, ","));
		numParameter = atoi(strtok(NULL, ","));
		sizeParameter = atoi(strtok(NULL, ","));
		valueParameter = strtok(NULL, ",");
		
		//Find the client
		if(numSystemCall != 1)
		{
			printf(">  SCANNING FOR CLIENT |\n");
			for(search = head; search->next->clientTag != clientPID; search = search->next)
			{
				if(search->next == NULL)
				{
					printf(">  ERROR : No hit.\n");
					return 1;
				}
				printf("                       | %d\n", search->next->clientTag);
			}
			printf("                   HIT | %d\n", search->next->clientTag);
		}
		
		printResults(numSystemCall, clientPID, numParameter, sizeParameter, valueParameter);

		switch(numSystemCall)
		{
			case -1:
				printf(">  Client %d requests server shutdown.\n", search->next->clientTag);
				
				write(search->next->writeFD, &numConnectedClients, 4);
				
				if(numConnectedClients == 1)
				{
					close(search->next->writeFD);
					
					free(head->next);
					free(head);
				
					printf(">  Server %s shutdown.\n", serverIn);
					return 0;
				}
				else
				{
					printf(">  ERROR : More than one client connected.\n");
					for(search = head; search->next != NULL; search = search->next)
					{
						printf("                       | %d\n", search->next->clientTag);
					}
				}
				
				break;
			case 0:
				close(search->next->writeFD);
				
				bridge = search->next;
				
				if(tail == bridge)
				{
					tail = search;
				}
				
				search->next = bridge->next;
				
				free(bridge);
				
				printf(">  Client %d shutdown.\n", clientPID);
				
				numConnectedClients--;
				break;
			case 1:
				tail->next = (node *) malloc(sizeof(node));
				tail->next->writeFD = open(valueParameter, O_WRONLY);

				printf(">  Connecting %s to %s.\n", valueParameter, serverIn);

				//Verify connection successful
				printf(">  PINGGING CLIENT : %d\n", clientPID);
				
				write(tail->next->writeFD, &numSystemCall, 4);
				
				//Store the client information in an array or list-structure
				

				tail->next->clientTag = clientPID;
				strncpy(tail->next->FIFO, valueParameter, 128);
				
				printf(">  NEW CLIENT :\n   | tag : %d | writeFD : %d |\n", tail->next->clientTag, tail->next->clientTag);
				
				tail = tail->next;

				numConnectedClients++;
				
				break;
			case 2:
				for( char * i = strtok(valueParameter, ";"); i != NULL; i = strtok(NULL, ";"))
				{
					integerStringConverter = atoi(i);
					write(search->next->writeFD, &integerStringConverter, 4);
					
				}
				break;
			case 3:
				for( char * i = strtok(valueParameter, ";"); i != NULL; i = strtok(NULL, ";"))
				{
					write(search->next->writeFD, i, MAXBUFFER);
				}
				break;
			case 4:
				storedValue = atoi(valueParameter);
				printf(">  STORED : %d\n", storedValue);
				break;
			case 5:
				write(search->next->writeFD, &storedValue, 4);
				printf(">  REQUESTED : %d\n", storedValue);
				break;
			default:
				printf(">  ERROR : Invalid request sent by client.\n");
				break;
		}
	}
	close(serverReadFD);
	close(serverWriteFD);
	unlink(serverIn);
	return 0;
}
