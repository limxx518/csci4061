#define _DEFAULT_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
//#include <zconf.h>
#include <arpa/inet.h>
#include <ctype.h>
//Additions
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <dirent.h>
#include <time.h>
#include <linux/limits.h>
#include "../include/protocol.h"

#define MAX_WORD_LENGTH	200

FILE *logfp;

void createLogFile(void) {
    pid_t p = fork();
    if (p==0)
        execl("/bin/rm", "rm", "-rf", "log", NULL);

    wait(NULL);
    mkdir("log", ACCESSPERMS);
    logfp = fopen("log/log_client.txt", "a");	//Edited to append mode
}

void mapper(char *str, int mapper_id, int sockfd){
	int letter_cnt[26] = {0};
	FILE *fp;		//file pointer for Mapper_i.txt
	FILE *tp;		//file pointer for text files in Mapper_i.txt
	char word[MAX_WORD_LENGTH];
	char path[PATH_MAX];
	char copy[PATH_MAX];
	int len;
	///New for this assignment
	int request_arr[REQUEST_MSG_SIZE];		//Request array's size is always fixed at 28
	int resp_arr_1[RESPONSE_MSG_SIZE];		//Short array for commands other than GET_AZLIST
	int resp_arr_2[LONG_RESPONSE_MSG_SIZE];	//Long array for GET_AZLIST commands
	int updateCalls = 0;
	
	memset(copy, 0, PATH_MAX);
	memset(resp_arr_2, 0, sizeof(resp_arr_2));
	
	strcat(copy, "MapperInput/");
	strcat(copy, str);
	if((fp = fopen(copy, "r")) == NULL){
		perror("Failed opening mapper file: ");
		exit(1);
	}
	//First, issue CHECKIN request to server
	memset(request_arr, 0, sizeof(request_arr));
	request_arr[RQS_COMMAND_ID] = CHECKIN;
	request_arr[RQS_MAPPER_PID] = mapper_id;
	printf("CHECKIN: Command: %d ID: %d\n", request_arr[RQS_COMMAND_ID], request_arr[RQS_MAPPER_PID]);
	write(sockfd, request_arr, sizeof(request_arr));	//TODO: Add code to make sure all bytes are written and read
	read(sockfd, resp_arr_1, sizeof(resp_arr_1));
	fprintf(logfp, "[%d] CHECKIN: %d %d\n", mapper_id, resp_arr_1[RSP_CODE], resp_arr_1[RSP_DATA]);
	memset(resp_arr_1, 0, sizeof(resp_arr_1));
	while(fgets(path, PATH_MAX, fp)){
		len = strlen(path);
		if(len > 0 && path[len-1] == '\n')
			path[len-1] = '\0';
		//Open each path listed in Mapper_i.txt
		if((tp = fopen(path, "r")) == NULL){
			printf("Failed opening text file %s\n", path);
			perror("Failed opening text file: ");
			printf("Mapper process killing parent and spawned process due to error\n");
			kill(0, SIGKILL);
			//exit(1);
		}
		//Run through all the words in text file and update letter_cnt
		while(fgets(word, MAX_WORD_LENGTH, tp)){
			char *ptr = word;
			if(!strcmp(word, " "))
				continue;
			while(!isalpha(*ptr) && *ptr != '\0')
				ptr++;
			if(*ptr >= 'A' && *ptr <= 'Z')	//uppercase
				letter_cnt[*ptr-0x41]++;
			else if(*ptr >= 'a' && *ptr <= 'z')		//lowercase
				letter_cnt[*ptr-0x61]++;
			}
			fclose(tp);
			updateCalls++;
			//Now, send an UPDATE_AZLIST request to server to update the result for recently processed text file
			memset(request_arr, 0, sizeof(request_arr));
			request_arr[RQS_COMMAND_ID] = UPDATE_AZLIST;
			request_arr[RQS_MAPPER_PID] = mapper_id;
			//Concatenate letter_cnt array with the request_arr
			memcpy(request_arr + 2, letter_cnt, 26*sizeof(int));
			printf("UPDATE_AZLIST: Command: %d ID: %d\n", request_arr[RQS_COMMAND_ID], request_arr[RQS_MAPPER_PID]); 
			write(sockfd, request_arr, sizeof(request_arr));	//TODO: Add code to make sure all bytes are written
			read(sockfd, resp_arr_1, sizeof(resp_arr_1));
			memset(letter_cnt, 0, sizeof(letter_cnt));
			memset(resp_arr_1, 0, sizeof(resp_arr_1));
		}
		//Print total number of messages sent to server to log file
		fprintf(logfp, "[%d] UPDATE_AZLIST: %d\n", mapper_id, updateCalls);
		//Proceed to issue the other request commands
		//GET_AZLIST command
		memset(request_arr, 0, sizeof(request_arr));
		request_arr[RQS_COMMAND_ID] = GET_AZLIST;
		request_arr[RQS_MAPPER_PID] = mapper_id;
		printf("GET_AZLIST: Command: %d ID: %d\n", request_arr[RQS_COMMAND_ID], request_arr[RQS_MAPPER_PID]);
		write(sockfd, request_arr, sizeof(request_arr));
		read(sockfd, resp_arr_2, sizeof(resp_arr_2));
		fprintf(logfp, "[%d] GET_AZLIST: %d ", mapper_id, resp_arr_2[RSP_CODE]);
		for(int i = 0; i < 26; i++)
			fprintf(logfp, "%d ", resp_arr_2[RSP_DATA+i]);
		fprintf(logfp, "\n");
		memset(resp_arr_2, 0, sizeof(resp_arr_2));
		//GET_MAPPER_UPDATES command
		request_arr[RQS_COMMAND_ID] = GET_MAPPER_UPDATES;
		printf("GET_MAPPER_UPDATES: Command: %d ID: %d\n", request_arr[RQS_COMMAND_ID], request_arr[RQS_MAPPER_PID]);
		write(sockfd, request_arr, sizeof(request_arr));
		read(sockfd, resp_arr_1, sizeof(resp_arr_1));
		fprintf(logfp, "[%d] GET_MAPPER_UPDATES: %d %d\n", mapper_id, resp_arr_1[RSP_CODE], resp_arr_1[RSP_DATA]);
		memset(resp_arr_1, 0, sizeof(resp_arr_1));
		//GET_ALL_UPDATES command
		request_arr[RQS_COMMAND_ID] = GET_ALL_UPDATES;
		printf("GET_ALL_UPDATES: Command: %d ID: %d\n", request_arr[RQS_COMMAND_ID], request_arr[RQS_MAPPER_PID]);
		write(sockfd, request_arr, sizeof(request_arr));
		read(sockfd, resp_arr_1, sizeof(resp_arr_1));
		fprintf(logfp, "[%d] GET_ALL_UPDATES: %d %d\n", mapper_id, resp_arr_1[RSP_CODE], resp_arr_1[RSP_DATA]);
		memset(resp_arr_1, 0, sizeof(resp_arr_1));
		//CHECKOUT command
		request_arr[RQS_COMMAND_ID] = CHECKOUT;
		printf("CHECKOUT: Command: %d ID: %d\n", request_arr[RQS_COMMAND_ID], request_arr[RQS_MAPPER_PID]);
		write(sockfd, request_arr, sizeof(request_arr));
		read(sockfd, resp_arr_1, sizeof(resp_arr_1));

		//Print to log, close TCP connection and exit
		fprintf(logfp, "[%d] CHECKOUT: %d %d\n", mapper_id, resp_arr_1[RSP_CODE], resp_arr_1[RSP_DATA]);
		fprintf(logfp, "[%d] close connection\n", mapper_id);
		close(sockfd);
}
		

int main(int argc, char *argv[]) {
    int mappers;
    char folderName[100] = {'\0'};
    char *server_ip;
    int server_port;
    
    //My own variables
    pid_t clients[MAX_MAPPER_PER_MASTER];
    pid_t wpid;
    int status;
    char str[20];
    struct sockaddr_in serv_addr;
    int sockfd[MAX_MAPPER_PER_MASTER];
    int masterfd;
    int request_arr[REQUEST_MSG_SIZE];
    int resp_arr_1[RESPONSE_MSG_SIZE];
    int resp_arr_2[LONG_RESPONSE_MSG_SIZE];

    if(argc == 5){ // 4 arguments
        strcpy(folderName, argv[1]);
        mappers = atoi(argv[2]);
        server_ip = argv[3];
        server_port = atoi(argv[4]);
        //Some error checking....
        if (mappers > MAX_MAPPER_PER_MASTER) {
            printf("Maximum number of mappers is %d.\n", MAX_MAPPER_PER_MASTER);
            printf("./client <Folder Name> <# of mappers> <server IP> <server Port>\n");
            exit(1);
        }
        else if(mappers <= 0){
			printf("Number of mappers mmust be > 0\n");
			exit(1);
		}
		//Error checking for port number
		char *test = argv[4];
		while(*test != '\0'){
			if(isalpha(*test) || *test == '-'){
				printf("Only digits allowed to specify port number\n");
				exit(1);
			}
			else
				test++;
		}
    } 
    else{
        printf("Invalid or less number of arguments provided\n");
        printf("./client <Folder Name> <# of mappers> <server IP> <server Port>\n");
        exit(1);
    }
    
    //Initialize struct for socket setup and connection
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(server_port);
    serv_addr.sin_addr.s_addr = inet_addr(server_ip);		///FOR NOW, use LOOPBACK. To be replaced with server_ip
    
    //Establish a TCP/IP connection with server
    for(int i=0; i < mappers; i++){
		sockfd[i] = socket(AF_INET, SOCK_STREAM, 0);
		printf("sockfd[%d] is %d\n", i, sockfd[i]);
	}

    // create log file
    createLogFile();

    // phase1 - File Path Partitioning
    traverseFS(mappers, folderName);

    // Phase2 - Mapper Clients's Deterministic Request Handling
	
	//First step: Fork the processes
	for(int i=0; i < mappers; i++){
		if((clients[i] = fork()) < 0){
			perror("Error forking");
			printf("Mapper process killing parent and spawned process due to error\n");
			kill(0, SIGKILL);
			//exit(1);
		}
		//Next, have each process call the mapper() function, where the bulk of work is done
		else if(!clients[i]){		//child processes(mappers)
			//Establish a TCP/IP connection with server
			//sockfd[i] = socket(AF_INET, SOCK_STREAM, 0);
			//printf("sockfd[%d] is %d\n", i, sockfd[i]);
			/*if(sockfd[i] < 0){
				perror("Error establishing socket: ");
				exit(1);
			}*/
			//Connect
			if(connect(sockfd[i], (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0){
				perror("Error connecting to server");
				printf("Mapper process killing parent and spawned process due to error\n");
				kill(0, SIGKILL);
				//exit(1);
			}
			fprintf(logfp, "[%d] open connection\n", i+1);
			//Now, connection is established. Each client calls mapper() function to process txt files
			//and communicate results to server
			memset(str, 0, sizeof(str));
			sprintf(str, "Mapper_%d.txt", i+1);
			printf("Client %d calling mappper()\n", i+1);
			printf("Sending sockfd[%d]: %d\n", i, sockfd[i]);
			mapper(str, i+1, sockfd[i]);
			exit(0);	//Prevent processes from executing subsequent code in main()
		}
		/*else{	//Master process should close all ends of their sockfd
			for(int i = 0; i < MAX_MAPPER_PER_MASTER; i++)
				close(sockfd[i]);
			}*/
			
	}

    // Phase3 - Master Client's Dynamic Request Handling (Extra Credit)
    //Wait for all forked processes to complete execution
    while(mappers){
		wpid = wait(&status);
		printf("Client process with PID %ld exited with status %d.\n", (long)wpid, status);
		if(status) perror("Error");
		mappers--;
	}
	printf("All mappers have completed execution. This is master running\n");
	//Open commands.txt
	FILE *command;
	char buffer[5];
	if((command = fopen("commands.txt", "r")) == NULL){
		perror("Error opening commands.txt");
		exit(1);
	}
	request_arr[RQS_MAPPER_PID] = -1;
	
	//Local copy variables for azList and total updates, master client
	int masterCopy[ALPHABETSIZE] = {0};
	int totalUpdates = 0;
	
	while(fgets(buffer, sizeof(buffer), command)){
		//Establish connection with server
		masterfd = socket(AF_INET, SOCK_STREAM, 0);
		if(connect(masterfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0){
			perror("Error connecting to server: ");
			exit(1);
		}
		fprintf(logfp, "[%d] open connection\n", -1);
		request_arr[RQS_COMMAND_ID] = atoi(buffer);
		write(masterfd, request_arr, sizeof(request_arr));
		if(atoi(buffer) == GET_AZLIST){
			read(masterfd, resp_arr_2, sizeof(resp_arr_2));
			fprintf(logfp, "[-1] GET_AZLIST: %d ", resp_arr_2[RSP_CODE]);
			//fprintf(logfp, "<Final Result of azList>\n");
			for(int j = 0; j< ALPHABETSIZE; j++){
				//fprintf(logfp, "%c: ", 'A'+j);
				fprintf(logfp, "%d ", resp_arr_2[RSP_DATA+j]);
				masterCopy[j] = resp_arr_2[RSP_DATA+j];
			}
			fprintf(logfp, "\n");
			memset(resp_arr_2, 0, sizeof(resp_arr_2));
		}
		else if(atoi(buffer) == GET_ALL_UPDATES){
			read(masterfd, resp_arr_1, sizeof(resp_arr_1));
			//fprintf(logfp, "<Final Result of All Updates>\n");
			fprintf(logfp, "[-1] GET_ALL_UPDATES: %d %d\n", resp_arr_1[RSP_CODE], resp_arr_1[RSP_DATA]);
			//fprintf(logfp, "%d\n", resp_arr_1[RSP_DATA]);
			totalUpdates = resp_arr_1[RSP_DATA];
		}
		else{	//All the rest should elicit an error response
			read(masterfd, resp_arr_1, sizeof(resp_arr_1));
			switch(request_arr[RQS_COMMAND_ID]){
				case CHECKIN: fprintf(logfp, "[%d] CHECKIN: %d %d\n", request_arr[RQS_MAPPER_PID], resp_arr_1[RSP_CODE], resp_arr_1[RSP_DATA]);
							  break;
				case UPDATE_AZLIST: fprintf(logfp, "[%d] UPDATE_AZLIST: %d\n", request_arr[RQS_MAPPER_PID], resp_arr_1[RSP_DATA]);
									break;
				case GET_MAPPER_UPDATES: fprintf(logfp, "[%d] GET_MAPPER_UPDATES: %d %d\n", request_arr[RQS_MAPPER_PID], resp_arr_1[RSP_CODE], resp_arr_1[RSP_DATA]);
										break;
				case CHECKOUT: fprintf(logfp, "[%d] CHECKOUT: %d %d\n", request_arr[RQS_MAPPER_PID], resp_arr_1[RSP_CODE], resp_arr_1[RSP_DATA]);
							   break;
				default: fprintf(logfp, "[%d] wrong command\n", request_arr[RQS_MAPPER_PID]);
						 break;
			}
		}
		memset(resp_arr_1, 0, sizeof(resp_arr_1));
		memset(buffer, 0, sizeof(buffer));
		close(masterfd);
		fprintf(logfp, "[-1] close connection\n");
	}
	//Print final results
	fprintf(logfp, "<Final Result of All Updates>\n");
	fprintf(logfp, "%d\n", totalUpdates);
	fprintf(logfp, "<Final Result of azList>\n");
	for(int j = 0; j< ALPHABETSIZE; j++){
		fprintf(logfp, "%c: ", 'A'+j);
		fprintf(logfp, "%d\n", masterCopy[j]);
	}
	fprintf(logfp, "\n");
    fclose(logfp);
    return 0;
}
