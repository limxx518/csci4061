#define _DEFAULT_SOURCE
#include "phase2_3.h"
// You are free to use your own logic. The following points are just for getting started
/* 	Map Function
	1)	Each mapper selects a Mapper_i.txt to work with
	2)	Creates a list of letter, wordcount from the text files found in the Mapper_i.txt
	3)	Send the list to Reducer via pipes with proper closing of ends
*/
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <dirent.h>
#include <string.h>
#include <time.h>
#include <ctype.h>
#include <linux/limits.h>	//for the PATH_MAX #define statement

#define MAX_WORD_LENGTH 	200

int mapper(char *str, int mapper_id, int fd[][2])
{
	unsigned int letter_cnt[26] = {0};
	int flag;
	FILE *fp;
	FILE *tp;
	char word[MAX_WORD_LENGTH];
	char path[PATH_MAX];
	char copy[PATH_MAX];
	int len;
	
	memset(copy, 0, PATH_MAX);
	
	strcat(copy, "MapperInput/");
	strcat(copy, str);
	if((fp = fopen(copy, "r")) == NULL)
	{
		perror("Failed opening mapper file: ");
		return -1;
	}
	while(fgets(path, PATH_MAX, fp))	//get each path to txt file
	{
		len = strlen(path);
		if(len > 0 && path[len-1] == '\n')		//this was a tricky bug to fix, in processing the strings in pathlist,
			path[len-1] = '\0';					//a '\n' char was added which cause fopen call to fail
		if((tp = fopen(path, "r")) == NULL)
		{
			printf("Failed opening text file %s\n", path);
			perror("Failed opening text file: ");
			return -1;
		}		
		while(fgets(word, MAX_WORD_LENGTH, tp))
		{
			char* ptr = word;
			if(!strcmp(word, " "))
				continue;
			while(!isalpha(*ptr) && *ptr != '\0')
				ptr++;
			if(*ptr >= 'A' && *ptr <= 'Z')	//uppercase
				letter_cnt[*ptr-0x41]++;
			else if(*ptr >= 'a' && *ptr <= 'z')	//lowercase
				letter_cnt[*ptr-0x61]++;
		}
		fclose(tp);		//close the text file
	}
	fclose(fp);
	//while loop to ensure all data from letter_cnt is written to pipe
	while((flag = write(fd[mapper_id][1], letter_cnt, sizeof(letter_cnt))) < sizeof(letter_cnt));	
	if(flag == -1)
	{
		perror("Error writing to pipe: ");
		return -1;
	}
	//all data is written, close the write end of pipe so that read can check for EOF and terminate its end of while loop
	close(fd[mapper_id][1]);
	return 0;
}	

int phase2_3(int num_fork)
{
	char str[20];
	unsigned int letter_cnt[26] = {0};
	int buf[26];
	pid_t pid[num_fork+1];
	pid_t wpid;
	int status;
	int fd[num_fork][2];
	FILE* reducer;
	
	for(int i = 0; i < num_fork; i++)	//Create num_fork pipes
		pipe(fd[i]);	
	for (int i = 0; i < num_fork+1; i++)	//Spawn the mapper processes and reduce process
	{
		if((pid[i] = fork()) < 0)
		{
			perror("Error forking:");
			return -1;
		}
		else if(!pid[i] && (i != num_fork))	//mapper process
		{
			//close irrelevant fd streams
			for(int j = 0; j < num_fork; j++)
			{
				if(j!=i)		//close both read and write streams, not the mapper's interest
				{
					close(fd[j][0]);
					close(fd[j][1]);
				}
				else            //close the read stream only, since we want to write to reducer
					close(fd[j][0]);
			}
			sprintf(str, "Mapper_%d.txt", i);
			mapper(str, i, fd);
			exit(0);			//to prevent mapper process from spawning further children
		}
		else if(i == num_fork && pid[i]==0)
		{
			for(int j = 0; j < num_fork; j++)	//close all the write streams, since reducer only wants to read from mappers
				close(fd[j][1]);
			for(int j = 0; j < num_fork; j++)
			{
				//printf("Read result from mapper %d: \n", j);	
				while((read(fd[j][0], buf, sizeof(buf))) >0);	//poll EOF to get all data from pipe from a mapper
				//while(read(fd[j][0], buf, sizeof(buf)) != EOF);	//This somehow doesn't work, not sure what error in logic
				//NOTE: Cause found, read() returns 0 when there's nothing left to read, not EOF
				for(int k = 0; k < 26; k++)
				{
					//printf("%c %d\n", k+0x41, buf[k]);
					letter_cnt[k] += buf[k];
				}
				close(fd[j][0]);		//done reading from that mapper pipe's end
			}
			//Print results to ReducerResult.txt	
			if((reducer = fopen("ReducerResult.txt", "a")) == NULL)
			{
				perror("Error creating ReducerResult.txt: ");
				exit(-1);
			}
			for(int j = 0; j < 26; j++)
				fprintf(reducer, "%c %d\n", j+0x41, letter_cnt[j]);
			fclose(reducer); 
			exit(0);				
		}	
	}	
	//Parent process to close all their ends of pipes and wait for all children to finish execution
	for(int i = 0; i < num_fork; i++)
	{
		close(fd[i][0]);
		close(fd[i][1]);
	}
	while(num_fork > -1)
	{
		wpid = wait(&status);
		printf("Child with PID %ld exited with status %d.\n", (long)wpid, status);
		num_fork--;
	}
	return 0;	
}


		
