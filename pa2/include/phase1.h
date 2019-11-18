// Add Guard to header file
#ifndef PHASE1_H
#define PHASE1_H
// Function prototypes to 
//			Traverse the Folder
//			Partition the text file paths to 'm' files

int traverse_dir(char *path);		//This function traverses through the directory structure till it reaches text files
int delegate(int map_num);			//This function evenly delegates the file paths to each mapper in Mapper_i.txt in round robin fashion
int phase1(char *path, int map_num);//phase1 is a wrapper function for the above two functions

#endif
