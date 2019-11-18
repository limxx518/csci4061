    /* test machine : csel-kh1250-22.cselabs.umn.edu
    * date : 10/18/19
    * name : Jamal Khan, Li-Sha Lim
    * x500 : limxx518, khanx090
    */

## The Purpose of the Program / What the Program Does

This program aims to imitate certain aspects of the core functionality of [Google's MapReduce](https://ai.google/research/pubs/pub62) programming model on a small localized dataset. Google's MapReduce serves as a programming model that can process very large datasets in an efficient and distributed fashion. See previous URL and PDF for more info on Google's MapReduce.

In our program, we take a folder/directory containing any number of subfolders and text files as an input, and a number which represents the amount of mapper processes that will process these files. The program then traverses into this folder, recursing through its entire hierarchy to note all the files present. The mapper processes then divide and conquer, splitting up the file processing equally among the user specified number of mapping processes as it parses the files. In each text file, there is to be a single word per line. The program then counts the first letter of each word, and keeps a running tally of all 26 letters' count across all the files for each mapper. The reducer process is sent the alphabet counts from each mapper, where it is then aggregated into one final alphabet count, which is then outputted into a ```FinalResult.txt``` file.

Our program uses process spawning via pipes and forks, file I/O operations, and redirection to accomplish these tasks, imitating Google's Map phase in the early phases of our program, and then the Reduce phase in the latter phases.

## How to Compile & Run the Program
In your terminal, navigate to the pa2 directory and use the command ```make``` to compile the program for the first time, or ```make clean;make``` if you want to run the program after the first time. This will create an executable file called ```mapreduce```, which requires 2 arguments:

```shell
./mapreduce folderName numberOfMappers
```
- Where ```folderName``` is the root directory to be traversed by the program.
- Where ```numberOfMappers``` is the desired number of mapper processes used in the program. This number has to be an integer greater than 0 but less than 33.

## Assumptions outside what is mentioned in the assignment pdf
- The only type of files contained in the directory specified at the commandline are either subdirectories or files formatted such that there is only one word per line with the normal 26 english alphabetical characters ``a-z`` or ```A-Z```.
- Delimiters used on files are linux friendly (```\n```). i.e. there are no guarantees that the program supports Windows style delimiters (e.g. ```\r```).
- Any grading/testing scripts will have to call ```make clean;make``` (or equivilant) in between calls to the ```mapreduce``` executable (as per TA, this was okayed in advance). The concern is that iff ```mapreduce``` is called consequtively without a clean ```src``` directory, ```ReducerResult.txt``` and ```FinalResult.txt``` may have 26 additional lines appended to the file (or other unexpected behavior not defined in PDF) with each call to ```mapreduce```.
- We integrated phase3 functionality in phase2 while still keeping the program relatively modular. The act of keeping phase3 functionality in it's own .h and .c files would introduce obfuscation due to how intertwined phase2 & phase3 are in our implementation for this particular. Thus, we merged phase2 & phase3 functionality into a ```phase2_3.h``` and ```phase2_3.c``` files.
- To fulfill extra credit portion, we can skip over symbolic links as detailed by the TA here: https://canvas.umn.edu/courses/135130/discussion_topics/488856
- Unlike lab6, I removed the max argument in my process_dir() function. Thus, multi-level folder traversal recurses indefinitely, i.e. there is no integer such as "max" which can limit the recursion level. It is up to the user to make sure the system resources can handle the recursion involved of the ```folderName``` they opt for. We could limit it to 500 files as the PDF seems to hint at, but it does not appear to explicitly require us to do that, so we won't.
- Empty files count as a file.
- Symbolic links do not appear to be guaranteed to be portable across different filesystems and DVCS. It is up to the user to make sure symbolic links are in a correct state (e.g. actually a soft link and not a text file), otherwise they may be treated like a text file.
- Using PATH_MAX constant via ```#include <linux/limits.h>``` to limit length of filepaths.
- The user should take care not to specify a directory that has files other than folders or text files.

## Team names and x500s
Team name: team0
x500s: limxx518, khanx090

## Your and your partners individual contributions
We both did the project seperately so as to adequately learn the coursework, but we collaborated as we went along, discussing the challenges involved. Some portions of our programs are collaborations (like piping & forking, and other random bits like handling alphabets). But both instances of our programs passed all the tests. So we each completed every task required of the assignment. We submitted each of our assignments seperately to get vital feedback on if we are approaching the materials properly.

## If you have attempted extra credit
Yes.