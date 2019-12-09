#define _DEFAULT_SOURCE
#include <stdio.h>
#include <netdb.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
//#include <zconf.h>
#include <pthread.h>
#include <signal.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <ctype.h>
#include "../include/protocol.h"

//Define statements for updateStatus table
#define MAPPERID 0
#define NUMUPDATES 1
#define CHECKINOUT	2

typedef struct threadArg{
	int clientfd;
	char * clientip;
	int clientport;
	int index;
}threadArg;

//Globals
int azList[26] = {0};
int updateStatus[MAX_STATUS_TABLE_LINES][3] = {0};
int currentConn = 0;
int highestIndex = 0;		//Could use this variable to optimize GET_ALL_UPDATES
int occupied[MAX_CONCURRENT_CLIENTS] = {0};
pthread_mutex_t letterList = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t statusTable = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t connCount = PTHREAD_MUTEX_INITIALIZER;

void *service(void *arg){
	threadArg *argPtr = (threadArg *)arg;
	int socketfd = argPtr->clientfd;
	printf("Thread PID %ld with socketfd: %d\n", pthread_self(), socketfd);
	int req[REQUEST_MSG_SIZE] = {0};
	int result_1[RESPONSE_MSG_SIZE] = {0};
	int result_2[LONG_RESPONSE_MSG_SIZE] = {0};
	int run = 1;
	//Read requests issued from clients through socket and handle them
	//Run continuously until a CHECKOUT command is issued
	/*NOTES: For the updateStatus table, table is structures such as first column values hold a 0 or 1,
	 * 		 with 0 denoting non-existing entry for the mapper ID PID of value (row index + 1) and a 1
	 * 		 denoting an existing entry. The other two columns hold the values for #updates and check in/out
	 * 		 status for corresponding mappers */
	while(run){
		read(socketfd, req, sizeof(req));
		printf("Command: %d\n", req[RQS_COMMAND_ID]);
		printf("Client ID: %d\n", req[RQS_MAPPER_PID]);
		//First, check if mapper ID is valid
		if(req[RQS_MAPPER_PID] <= 0 && req[RQS_MAPPER_PID] != -1)	//Extra credit case
		{
			printf("Error: Request from invalid mapper ID %d\n", req[RQS_MAPPER_PID]);
			printf("Response code is %d\n", req[RQS_COMMAND_ID]);
			//while(1);
			result_1[RSP_COMMAND_ID] = req[RQS_COMMAND_ID];
			result_1[RSP_CODE] = RSP_NOK;
			write(socketfd, result_1, sizeof(result_1));
			memset(result_1, 0, sizeof(result_1));
			memset(req, 0, sizeof(req));
		}
		else{
			switch(req[RQS_COMMAND_ID]){
				case CHECKIN: printf("[%d] CHECKIN\n", req[RQS_MAPPER_PID]);
							  pthread_mutex_lock(&statusTable);
							  if(req[RQS_MAPPER_PID] != -1 && updateStatus[req[RQS_MAPPER_PID]-1][MAPPERID]){
								  //Check CHECKIN status
								  if(!updateStatus[req[RQS_MAPPER_PID]-1][CHECKINOUT]){
									updateStatus[req[RQS_MAPPER_PID]-1][CHECKINOUT] = 1;
									result_1[RSP_COMMAND_ID] = CHECKIN;
									result_1[RSP_CODE] = RSP_OK;
									result_1[RSP_DATA] = req[RQS_MAPPER_PID];
									write(socketfd, result_1, sizeof(result_1));
								}
								  else{		//Raise an error if already checked in
									  printf("Mapper client %d already checked in\n", req[RQS_MAPPER_PID]);
									  result_1[RSP_COMMAND_ID] = CHECKIN;
									  result_1[RSP_CODE] = RSP_NOK; 
									  result_1[RSP_DATA] = req[RQS_MAPPER_PID];
									  write(socketfd, result_1, sizeof(result_1));
								  }
								  pthread_mutex_unlock(&statusTable);
								 }
								  //No existing entry, therefore create new one
							  else if (req[RQS_MAPPER_PID] != -1){
								 updateStatus[req[RQS_MAPPER_PID]-1][MAPPERID] = 1;
								 updateStatus[req[RQS_MAPPER_PID]-1][CHECKINOUT] = 1;
								 pthread_mutex_unlock(&statusTable);
								 result_1[RSP_COMMAND_ID] = CHECKIN;
								 result_1[RSP_CODE] = RSP_OK;
								 result_1[RSP_DATA] = req[RQS_MAPPER_PID];
								 write(socketfd, result_1, sizeof(result_1));
								}
							  else{		//Master client shouldn't check in
								  pthread_mutex_unlock(&statusTable);
								  result_1[RSP_COMMAND_ID] = CHECKIN;
								  result_1[RSP_CODE] = RSP_NOK;
								  result_1[RSP_DATA] = req[RQS_MAPPER_PID];
								  write(socketfd, result_1, sizeof(result_1));
							  }
							  break;
				case UPDATE_AZLIST: //Check if client is checked in
									pthread_mutex_lock(&statusTable);
									if(req[RQS_MAPPER_PID] != -1 && !updateStatus[req[RQS_MAPPER_PID]-1][CHECKINOUT]){
										pthread_mutex_unlock(&statusTable);
										printf("Error, mapper client %d not checked in\n", req[RQS_MAPPER_PID]);
										result_1[RSP_COMMAND_ID] = UPDATE_AZLIST;
										result_1[RSP_CODE] = RSP_NOK;
										result_1[RSP_DATA] = req[RQS_MAPPER_PID];
										write(socketfd, result_1, sizeof(result_1));
									}
									else if(req[RQS_MAPPER_PID] != -1){
										updateStatus[req[RQS_MAPPER_PID]-1][NUMUPDATES]++;
										pthread_mutex_unlock(&statusTable);		//Do in this order to avoid possible deadlocks
										pthread_mutex_lock(&letterList);
										for(int i = 0; i < ALPHABETSIZE; i++)
											azList[i] += req[RQS_DATA+i];
										pthread_mutex_unlock(&letterList);
										result_1[RSP_COMMAND_ID] = UPDATE_AZLIST;
										result_1[RSP_CODE] = RSP_OK;
										result_1[RSP_DATA] = req[RQS_MAPPER_PID];
										write(socketfd, result_1, sizeof(result_1));
									}
									else{	//This command should be invalid for master client
										pthread_mutex_unlock(&statusTable);
										result_1[RSP_COMMAND_ID] = UPDATE_AZLIST;
										result_1[RSP_CODE] = RSP_NOK;
										result_1[RSP_DATA] = req[RQS_MAPPER_PID];
										write(socketfd, result_1, sizeof(result_1));
									}
									break;
			case GET_AZLIST: printf("[%d] GET_AZLIST\n", req[RQS_MAPPER_PID] && req[RQS_MAPPER_PID] != -1);
							 //check if client is checked in, if it's not master
							 pthread_mutex_lock(&statusTable);
							 if(req[RQS_MAPPER_PID] != -1 && !updateStatus[req[RQS_MAPPER_PID]-1][CHECKINOUT]){
								 pthread_mutex_unlock(&statusTable);
								 printf("Error, mapper client %d not checked in\n", req[RQS_MAPPER_PID]);
								 result_2[RSP_COMMAND_ID] = GET_AZLIST;
								 result_2[RSP_CODE] = RSP_NOK;
								 write(socketfd, result_2, sizeof(result_2));
							 }
							 else{
								 pthread_mutex_unlock(&statusTable);
								 pthread_mutex_lock(&letterList);
								 for(int i = 0; i < ALPHABETSIZE; i++)
									result_2[RSP_DATA+i] = azList[i];
								 pthread_mutex_unlock(&letterList);
								 result_2[RSP_COMMAND_ID] = GET_AZLIST;
								 result_2[RSP_CODE] = RSP_OK;
								 write(socketfd, result_2, sizeof(result_2));
							 }
							 break;
			case GET_MAPPER_UPDATES: printf("[%d] GET_MAPPER_UPDATES\n", req[RQS_MAPPER_PID]);
									 //check if client is checked in
									 pthread_mutex_lock(&statusTable);
									 if(req[RQS_MAPPER_PID] != -1 && !updateStatus[req[RQS_MAPPER_PID]-1][CHECKINOUT]){
									 	 pthread_mutex_unlock(&statusTable);
										 printf("Error, mapper client %d not checked in\n", req[RQS_MAPPER_PID]);
										 result_1[RSP_COMMAND_ID] = GET_MAPPER_UPDATES;
										 result_1[RSP_CODE] = RSP_NOK;
										 write(socketfd, result_1, sizeof(result_1));
									 }
									 else if(req[RQS_MAPPER_PID] != -1){	//This request is only valid for mapper clients
										 result_1[RSP_DATA] = updateStatus[req[RQS_MAPPER_PID]-1][NUMUPDATES];
										 pthread_mutex_unlock(&statusTable);
										 result_1[RSP_COMMAND_ID] = GET_MAPPER_UPDATES;
										 result_1[RSP_CODE] = RSP_OK;
										 write(socketfd, result_1, sizeof(result_1));
									 }
									 else{
										 pthread_mutex_unlock(&statusTable);
										 result_1[RSP_COMMAND_ID] = GET_MAPPER_UPDATES;
										 result_1[RSP_CODE] = RSP_NOK;
										 write(socketfd, result_1, sizeof(result_1));
									}
									 break;
			case GET_ALL_UPDATES: printf("[%d] GET_ALL_UPDATES\n", req[RQS_MAPPER_PID]);
								  //check if client is checked in
								  pthread_mutex_lock(&statusTable);
								  if(req[RQS_MAPPER_PID] != -1 && !updateStatus[req[RQS_MAPPER_PID]-1][CHECKINOUT]){
									  pthread_mutex_unlock(&statusTable);
									  printf("Error, mapper client %d not checked in\n", req[RQS_MAPPER_PID]);
									  result_1[RSP_COMMAND_ID] = GET_ALL_UPDATES;
									  result_1[RSP_CODE] = RSP_NOK;
									  write(socketfd, result_1, sizeof(result_1));
								  }
								  else{
									  for (int i = 0; i < MAX_STATUS_TABLE_LINES; i++){
										  if(updateStatus[i][MAPPERID])	//check if entry is valid
								              result_1[RSP_DATA] += updateStatus[i][NUMUPDATES];
									   }
									   pthread_mutex_unlock(&statusTable);
									   result_1[RSP_COMMAND_ID] = GET_ALL_UPDATES;
									   result_1[RSP_CODE] = RSP_OK;
									   write(socketfd, result_1, sizeof(result_1));
								   }
								   break;
			case CHECKOUT: printf("[%d] CHECKOUT\n", req[RQS_MAPPER_PID]);
						   //check if client is checked in
						   pthread_mutex_lock(&statusTable);
						   if(req[RQS_MAPPER_PID] != -1 && !updateStatus[req[RQS_MAPPER_PID]-1][CHECKINOUT]){
								pthread_mutex_unlock(&statusTable);
							    printf("Error, mapper client %d not checked in\n", req[RQS_MAPPER_PID]);
							    result_1[RSP_COMMAND_ID] = CHECKOUT;
							    result_1[RSP_CODE] = RSP_NOK;
								write(socketfd, result_1, sizeof(result_1));
							}
							else if(req[RQS_MAPPER_PID] != -1){
								updateStatus[req[RQS_MAPPER_PID]-1][CHECKINOUT] = 0;
								pthread_mutex_unlock(&statusTable);
								result_1[RSP_COMMAND_ID] = CHECKOUT;
								result_1[RSP_CODE] = RSP_OK;
								result_1[RSP_DATA] = req[RQS_MAPPER_PID];
								write(socketfd, result_1, sizeof(result_1));
								run = 0;
							}
							else{
								pthread_mutex_unlock(&statusTable);
								result_1[RSP_COMMAND_ID] = CHECKOUT;
								result_1[RSP_CODE] = RSP_NOK;
								result_1[RSP_DATA] = req[RQS_MAPPER_PID];
								write(socketfd, result_1, sizeof(result_1));
							}
							break;
		    default: //assume invalid request code
					 result_1[RSP_COMMAND_ID] = req[RQS_COMMAND_ID];
					 result_1[RSP_CODE] = RSP_NOK;
					 write(socketfd, result_1, sizeof(result_1));
					 break;
		}
		if(req[RQS_MAPPER_PID] == -1)
			run = 0;
		//Now reset buffers
		memset(result_1, 0, sizeof(result_1));
		memset(result_2, 0, sizeof(result_2));
		memset(req, 0, sizeof(req));
		}
	}
	//Client has closed its connection. Decrement currentConn and mark occupied[index] as available
	pthread_mutex_lock(&connCount);
	currentConn--;
	occupied[argPtr->index] = 0;
	pthread_mutex_unlock(&connCount);
	printf("close connection from %s:%d\n", argPtr->clientip, argPtr->clientport);
	close(socketfd);
	pthread_exit(NULL);
}
									

int main(int argc, char *argv[]) {

    int server_port;
    //Additional variables
    int sockfd, newsockfd;
    //int newsockfd[MAX_CONCURRENT_CLIENTS];
    struct sockaddr_in serv_addr, client_addr;
    pthread_t threads[MAX_CONCURRENT_CLIENTS];
    //int j = 0;		//index to add newsockfd
    //int k = 0;		//index to provide newsockfd to threads, should lag j at all times

    if (argc == 2) { // 1 arguments
		//Check validity of arg[1]
		char *test = argv[1];
		while(*test != '\0'){
			if(isalpha(*test) || *test == '-'){
				printf("Only digits allowed to specify port number\n");
				exit(1);
			}
			else
				test++;
		}
        server_port = atoi(argv[1]);
    } else {
        printf("Invalid or less number of arguments provided\n");
        printf("./server <server Port>\n");
        exit(0);
    }
    
    //Create a TCP socket and convert it to a listening port. This port is the entry point of connection
    //for all mapper clients
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if(sockfd < 0){
		perror("Error establishing socket on server end");
		exit(1);
	}
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port = htons(server_port);
	serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	//Bind socket to local address and port
	if(bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0){
		perror("Error on binding, server side");
		exit(1);
	}
	//Convert socket to a passive socket
	printf("server is listening\n");
	listen(sockfd, MAX_CONCURRENT_CLIENTS);
	socklen_t size = sizeof(struct sockaddr_in);

    // Server (Reducer) code
    //Have server running indefinitely until it is explicitly killed by user, (ie external signal)
    while(1){		
		newsockfd = accept(sockfd, (struct sockaddr *) &client_addr, &size);
		if(newsockfd < 0){
			perror("Error on accept");
			exit(1);
		}
		//Spawn a thread to handle the read and write requests for a particular mapper client
		threadArg args[MAX_CONCURRENT_CLIENTS];
		pthread_mutex_lock(&connCount);
		int i = 0;
		while(occupied[i]){
			i++;
			i = i%(MAX_CONCURRENT_CLIENTS);
		}
		occupied[i] = 1;
		args[i].index = i;
		args[i].clientfd = newsockfd;
		args[i].clientip = inet_ntoa(client_addr.sin_addr);
		args[i].clientport = client_addr.sin_port;
		printf("open connection from %s:%d\n", args[i].clientip, args[i].clientport);
		//pthread_mutex_lock(&connCount);
		if(currentConn >= MAX_CONCURRENT_CLIENTS){
			printf("Server has reached max capacity of serving 50 clients simultaneously\n");
			close(args[i--].clientfd);
			continue;
		}
		else{	//Spawn a thread to handle connection, and increase currentConn
			/*int i = 0;
			while(occupied[i]){
				i++;
				i = i%50;
			}
			occupied[i] = 1;
			args[j].index = i;*/
			printf("Spawning thread with sockfd: %d\n", args[i].clientfd);
			pthread_create(&threads[i], NULL, service, (void*)&args[i]);
			currentConn++;
		}
		pthread_mutex_unlock(&connCount);
	}
	
    return 0;
} 
