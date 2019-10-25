/*test machine: atlas.cselabs.umn.edu
* date: 10/8/19
* name: Jamal Khan, Li-Sha Lim
* x500: khanx090, limxx518
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/wait.h>
#include "globals.h"
	
pid_t child_pid[MAX_RECIPES_PT];

int main(int argc, char* argv[])
{
	int target_num;
	int test;
	
	if(argc > 3 || argc < 2)	//throw an error
	{
		if(argc > 3)
			printf("Max of 3 arguments expected\n");
		else
			printf("Minimum of 2 arguments expected\n");
		return -1;
	}
	
	if(argc == 3 && !strncmp(argv[1], "-p", 2))
	{
		test = process_file(argv[2]);
		if(test!=0)
		{
			printf("File %s doesn't exist\n", argv[2]);
			return -1;
		}
		target_num = parse_lines();
		for(int j=0; j< target_num; j++)
		{
			printf("Target '%s' has %d dependencies and %d recipe\n", targets[j].name, targets[j].dep_count, targets[j].recipe_count);
			for(int k=0; k < targets[j].dep_count; k++)
				printf("Dependency %d is %s\n", k, targets[j].depend[k]);
			for(int l=0; l < targets[j].recipe_count; l++)
				printf("Recipe %d is %s\n", l, targets[j].recipe[l]);
			printf("\n");
		}
	}
	//Strategy used to determine order of recipe execution is by forming an adjacency matrix with makefile targets being the
	//indices. Then use DFS on the adjacency matrix and insert the recipes into an array as the recursion unrolls.
	else if(argc == 3 && !strncmp(argv[1], "-r", 2))
	{
		test = process_file(argv[2]);
		if(test!=0)
		{
			printf("File %s doesn't exist\n", argv[2]);
			return -1;
		}
		target_num = parse_lines();
		create_adjacency_matrix(target_num);
		DFS(0, target_num);						//0 constant in our case because we start from first target by default
		printf("Execution order of recipes are:\n");
		for(int a=0; a<rCount; a++)
			printf("%s\n", ordered_recipes[a]);
	}
	else if(argc == 2)
	{
		pid_t pid, wpid;
		int a = 0;
		test = process_file(argv[1]);
		if(test!=0)
		{
			printf("File %s doesn't exist\n", argv[1]);
			return -1;
		}
		target_num = parse_lines();
		create_adjacency_matrix(target_num);
		DFS(0,target_num);
		printf("Execution order of recipes are:\n");
		for(int a=0; a<rCount; a++)
			printf("%s\n", ordered_recipes[a]);
		printf("\n");
		while(a<rCount)
		{
			int status;
			if(ordered_recipes_offset[a])
			{
				//printf("Parallel process execution\n");
				for(int b=0; b<ordered_recipes_offset[a]; b++)
					tokenize_arguments(ordered_recipes[b+a], b, 1);
				for(int b=0; b<ordered_recipes_offset[a]; b++)
				{
					if((pid = fork()) == 0)
					{
						//printf("Child process [pid]:%d forking %s\n", getpid(), ordered_recipes[b+a]);
						execvp(args2[b][0], args2[b]);
					}
					else if(pid<0)
					{
						perror("Error forking process\n");
						return -1;
					}
				}
				while((wpid = wait(&status)) > 0);		//Parent process waiting for all child processes to finish execution
				//printf("All child processes have completed execution\n");
				a += ordered_recipes_offset[a];
			}
			else
			{
				pid = fork();
				if(pid<0)
				{		
					perror("Error forking process\n");
					return -1;
				}
				else if(pid>0)
				{
					//printf("Parent process waiting for child to finish execution of %s\n", ordered_recipes[a]);
					wait(NULL);
					//printf("Done process, forking next one\n");
				}
				else
				{
					//printf("Child process executing %s\n", ordered_recipes[a]);
					tokenize_arguments(ordered_recipes[a], 0, 0);
					execvp(args[0], args);
				}
				a++;
			}
		}
	}
	else if(argc == 3)
	{
		int a=0, x=0;
		pid_t pid, wpid;
		
		test = process_file(argv[1]);
		if(test!=0)
		{
			printf("File %s doesn't exist\n", argv[1]);
			return -1;
		}
		if((strncmp(argv[2], "-p", 2)==0) || (strncmp(argv[2], "-r", 2)==0))
		{
			printf("Incorrect order of arguments entered\n");
			return -1;
		}
		target_num = parse_lines();
		create_adjacency_matrix(target_num);
		for(x=0; x<target_num; x++)
		{
			if(!strcmp(targets[x].name, argv[2]))
			{	
				DFS(x, target_num);
				break;
			}
		}
		if(x==target_num)
		{
			printf("Target %s doesn't exist\n", argv[2]);
			return -1;
		}
		printf("Execution order of recipes are:\n");
		for(int a=0; a<rCount; a++)
			printf("%s\n", ordered_recipes[a]);
		printf("\n");
		while(a<rCount)
		{
			int status;
			if(ordered_recipes_offset[a])
			{
				//printf("Parallel process execution\n");
				//printf("Value of a is %d\n", a);
				//printf("Ordered_recipes_offset is %d\n", ordered_recipes_offset[a]);
				for(int b=0; b<ordered_recipes_offset[a]; b++)
					tokenize_arguments(ordered_recipes[b+a], b, 1);
				for(int b=0; b<ordered_recipes_offset[a]; b++)
				{
					if((pid = fork()) == 0)
					{
						//printf("Child process [pid]:%d forking %s\n", getpid(), ordered_recipes[b+a]);
						execvp(args2[b][0], args2[b]);
					}
					else if(pid<0)
					{
						perror("Error forking process\n");
						return -1;
					}
				}
				while((wpid = wait(&status)) > 0);		//Parent process waiting for all child processes to finish execution
				//printf("All child processes have completed execution\n");
				a += ordered_recipes_offset[a];
			}
			else
			{
				pid = fork();
				if(pid<0)	
				{	
					perror("Error forking process\n");
					return -1;
				}
				else if(pid>0)
				{
					//printf("Parent process waiting for child to finish execution of %s\n", ordered_recipes[a]);
					wait(NULL);
					//printf("Done process, forking next one\n");
				}
				else
				{
					//printf("Child process executing %s\n", ordered_recipes[a]);
					tokenize_arguments(ordered_recipes[a], 0, 0);
					execvp(args[0], args);
				}
				a++;
			}
		}
	}
	return 0;
}
			
		
	
