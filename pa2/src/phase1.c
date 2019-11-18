#define _DEFAULT_SOURCE

#include "phase1.h"

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
#include <linux/limits.h>	//for the PATH_MAX #define statement

// You are free to use your own logic. The following points are just for getting started
/* 	Data Partitioning Phase - Only Master process involved
	1) 	Create 'MapperInput' folder
	2) 	Traverse the 'Sample' folder hierarchy and insert the text file paths
		to Mapper_i.txt in a load balanced manner
	3) 	Ensure to keep track of the number of text files for empty folder condition 

*/

///Global variables
int txt_file_num;
FILE *pathlist;

int traverse_dir(char *path)
{
	struct dirent *dirp;
	struct stat check;
	DIR *dir;
	char buf[PATH_MAX];
	strcpy(buf, path);
	
	if((dir = opendir(path)) == NULL)		//Open the directory
	{
		perror("Failed to open directory: ");
		return -1;
	}
	while((dirp = readdir(dir)) != NULL)
	{
		memset(buf, 0, sizeof(buf));
		strcat(buf, path);
		strcat(buf, "/");
		strcat(buf, dirp->d_name);
		if(lstat(buf, &check) == -1)
			perror("Stat error: ");
		if(S_ISLNK(check.st_mode))			//if file/dir is symbolic link ignore and go to next directory entry
			continue;
		if(!strcmp(dirp->d_name, ".") || !strcmp(dirp->d_name, ".."))	//ignore current and parent directories
			continue;
		else if(S_ISDIR(check.st_mode))	//entry is a folder, so recurse through that folder
			traverse_dir(buf);
		else if(S_ISREG(check.st_mode))	//entry is a regular file, so add the path to pathlist.txt, and increment text file num
		{
			if((pathlist = fopen("pathlist.txt", "a")) == NULL)
			{
				perror("Failed opening pathlist.txt: ");
				return -1;
			}
			fprintf(pathlist, "%s\n", buf);
			txt_file_num++;
			fclose(pathlist);
		}
	}
	if(!txt_file_num)	//empty directory
	{
		printf("Empty directory\n");
		remove("MapperInput");
		return -1;
	}
	else return 0;
}

int delegate(int map_num)
{
	int dividend, remainder;
	char line[PATH_MAX];
	char str[20];
	FILE *map;
	
	memset(str, 0, 20);
	
	if((pathlist = fopen("../pathlist.txt", "r")) == NULL)	//pathlist is stored in parent directory above MapperInput
	{
		perror("Failed opening pathlist.txt: ");
		return -1;
	}
	dividend = txt_file_num / map_num;
	remainder = txt_file_num % map_num;
	for(int i = 0; i < map_num; i++)
	{
		sprintf(str, "Mapper_%d.txt", i);
		if((map = fopen(str, "a")) == NULL)
		{
			perror("Failed opening text file: ");
			return -1;
		}
		for(int j = 0; j < dividend; j++)
		{
			fgets(line, PATH_MAX, pathlist);
			fprintf(map, "%s", line);
		}
		fclose(map);
	}
	if(remainder)	//There's uneven division, so fill the remainders in round-robin fashion, ie. one insertion per txt file
	{
		for(int i = 0; i < remainder; i++)
		{
			sprintf(str, "Mapper_%d.txt", i);
			if((map = fopen(str, "a")) == NULL)
			{
				perror("Failed opening text file: ");
				return -1;
			}
			fgets(line, PATH_MAX, pathlist);
			fprintf(map, "%s", line);
			fclose(map);
		}
	}
	fclose(pathlist);
	//Remove pathlist.txt file when outputs in Mapper_m files are filled in, not needed anymore
	if(remove("../pathlist.txt"))
		perror("Error deleting pathlist.txt: ");
	return 0;
}

int phase1(char *path, int map_num)
{
	char buf[PATH_MAX];
	DIR *dir;
	
	//Check if path to directory exists before creating MapperInput directory	
	if((dir = opendir(path)) == NULL)		//Open the directory
	{
		perror("Failed to open directory: ");
		return -1;
	}
	
	if(getcwd(buf, sizeof(buf)) == NULL)	//get current working directory
	{
		perror("Failed to get current working directory: ");
		return -1;
	}
	if(mkdir(strcat(buf, "/MapperInput"), S_IRWXU  | S_IRWXG | S_IROTH | S_IXOTH) == -1)	//create MapperInput dir
	{
		perror("Failed to create MapperInput directory: ");
		return -1;
	}
	
	if(!traverse_dir(path))	
	{
		if(chdir(buf) == -1)	//Change working directory to ..../MapperInput to store .txt files
		{
			perror("Error changing directory: ");
			return -1;
		}
		if(delegate(map_num) == -1)
			return -1;
	}
	else
		return -1;
	chdir("..");	//change working directory back up one level
	getcwd(buf, sizeof(buf));
	return 0;
}
	
	
