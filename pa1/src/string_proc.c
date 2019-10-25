#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/wait.h>
#include "globals.h"

//State definitions for parsing
#define SEARCH_TARGET 0
#define SEARCH_DEP	  1
#define SEARCH_REC	  2
#define ERROR		  3

char lines[MAX_LINES][LINE_SIZE];
int i = 0;
target targets[MAX_TARGETS];
char* args[LINE_SIZE];
char* args2[MAX_RECIPES_PT][LINE_SIZE];

//Parse input makefile to determine targets, dependencies and recipes
int process_file(char *fname)
{
	char line[LINE_SIZE];
	
	FILE* fp =fopen(fname, "r");
	if(!fp)
	{
		printf("Failed to open the file: %s \n", fname);
		return -1;
	}
	
	//Read the contents and store in lines
	while(fgets(line, LINE_SIZE, fp))
	{
		strncpy(lines[i++], line, strlen(line));
	}
	fclose(fp);
	
	return 0;
}

int parse_lines(void)
{
	char* test;
	char* token;
	const char delim[] = ": ";
	int j = 0;		//lines index
	int k = 0;		//struct index
	int l = 0; 		//dependencies index
	int m = 0;		//recipes index
	int n = 0;		//parallel_ct index
	
	int currentState = SEARCH_TARGET;
	
	while(j < i)		
	{
		switch(currentState)  
		{
			case SEARCH_TARGET: //Search the string if the ':' char is present, this denotes specification of a new target
								//Assumption here is that there are no empty string targets
								test = strchr(lines[j], ':');
								if(test != NULL)
								{
									token = strtok(lines[j], delim);
									targets[k].name = token;
									token = strtok(NULL, ": \n");
									if(token != NULL)
									{
										while(token != NULL)
										{
											targets[k].depend[l++] = token;
											targets[k].dep_count++;
											token = strtok(NULL, ": \n");
										}
									}
									else 
									{
										targets[k].depend[l++] = "";	//empty string to denote no dependencies
									}
									currentState = SEARCH_DEP;
								}
								
								break;
								
			case SEARCH_DEP: //Check the string to see if a tab is present, If true, go to SEARCH_REC state, else add dependencies
							 //Assumption here is there are no blank lines in a rule
							test = strchr(lines[j], '\t');
							if( test!= NULL) 
							{
									currentState = SEARCH_REC;
									j--;		//to avoid skipping a line since j is updated every switch case cycle
									l = 0;		//reset dependency index for next target element
							}
							else if(!strncmp(lines[j], "\n", 1));
							else if ((strchr(lines[j], ':')) != NULL)
							{
								//Presence of colon indicates a new rule, so next state should be SEARCH_TARGET
								k++;			//advance index to new target element
								l = 0;			//reset dependency index for next target element
								m = 0;			//reset recipe index for new target
								j--;			//to avoid skipping a line since j is updated every switch case cycle
								currentState = SEARCH_TARGET;
							}
							else
							{
								token = strtok(lines[j], " \n");
								targets[k].depend[l++] = token;
								targets[k].dep_count++;
								while(token != NULL)
								{
									targets[k].depend[l++] = token;
									targets[k].dep_count++;
									token = strtok(NULL, " \n");
								}
							}
							break;
							
		    case SEARCH_REC: //Writing this function to work for multiple recipes. Approach is to check for presence of 
							 // ':' character that signifies a new target specification, else continue adding recipes
							 test = strchr(lines[j], ':');
							 if(test != NULL)
							 {
								 targets[k].parallel_ct_tail = n;
								 n = 0;			//reset parallel_ct index for new target
								 k++;			//advance index to new target element
								 m = 0;			//reset recipe index for new target
								 j--;
								 currentState = SEARCH_TARGET;
							 }
							 else if(!strncmp(lines[j], "\n", 1));	//ignore blank lines
							 else 
							 {
								 //Assumption is there is a tab char present
								 test = strchr(lines[j], ',');
								 if(test)
								 {
									 int x = 0;
									 token = strtok(lines[j], "\t,\n");
									 while(token != NULL)
									 {									
										 targets[k].recipe[m++] = token;
										 targets[k].recipe_count++;
										 //this array keeps track of the number of recipes to be executed in parallel
										 targets[k].parallel_ct[n]++;		
										 token = strtok(NULL, ",\n");
										 if(token != NULL)
									     {
											 while(token[x] == ' ')
												x++;
											 token += x;
											 x = 0;
										 }
									 
									 }
								 }
								 else
								 {
									 token = strtok(lines[j], "\t\n");
									 targets[k].recipe_count++;
									 targets[k].recipe[m++] = token;
								 }
								 if(currentState != SEARCH_TARGET) n++;		//index for parallel_ct advanced evey iteration in this state
							 }
							 break;
							 
			//case ERROR: break;
		}
		j++;
	}
	targets[k].parallel_ct_tail = n;		//Account for EOF case
	return k+1;
}

void tokenize_arguments(char* str, int num, int bool)
{
	char* token;
	int a = 0;
	
	token = strtok(str, " ");
	if(!bool)
	{
		while(token!=NULL)
		{
			args[a++] = token;
			token = strtok(NULL, " ");
		}
		args[a] = (char*)NULL;
	}
	else
	{
		while(token!=NULL)
		{
			args2[num][a++] = token;
			token = strtok(NULL, " ");
		}
		args2[num][a] = (char*)NULL;
	}
}
