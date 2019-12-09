```
	/* test machine: csel-kh1250-02.cselabs.umn.edu
	 * date: 11/3/19
	 * name: Li-Sha Lim, Jamal Khan
	 * x500: limxx518, khanx090
	 */
```

## The Purpose of the Program / What the Program Does
The program takes an input file with english words and then counts the first letter of each word and outputs the result to either stdout and/or an output file ```results.txt```. The program uses POSIX pthreads to handle parsing of the input file in a line by line fashion in a "producer" thread, where the lines are read into a shared global queue (linked list). Using n number of "consumer" threads (the user can define the number of "consumer" threads at the command line as an argument), the program then parses the lines stored in the queue in an interleaving fashion as the producer and consumers all work concurrently, storing the "first letter counts" into a global histogram/array. The program ensures synchronization between the producer thread and the consumer threads by using mutual exclusion locks to ensure against race conditions and deadlocks.

## How to Compile & Run the Program
In your terminal, navigate to the pa3 directory and use the command ```make``` to compile the program. ```make clean``` restores the pa3 directory to a state without the executable or object files.

As per PDF, the program will take the arguments syntax:
```console
$ ./wcs #consumer filename [option] [#queue_size]
```
- The second argument “#consumer” is the number of consumers the
program will create.
- The third argument “filename” is the input file name
- Options have only three possibilities: “-p”, “-b”, “-bp”.
    - “-p” means printing, the program will generate log in this case.
    - “-b” means bounded buffer (extra credit), the program will use it
instead of unbounded buffer.
    - “-bp” means both bounded buffer and log printing.
- The last option argument is the queue size if using bounded buffer (extra
credits).

## Assumptions outside what is mentioned in the assignment PDF
- Program is compatible in linux only.
- It is the duty of the the user to ensure the machine can handle the resources required by the input file.
- The program only parses english words, where a word is defined as a continuous A-Z or a-z character(s) instance/sequence.
- The number of consumers is limited to the range of ```int```.

## Team names and x500s
Team name: team0
x500s: limxx518, khanx090

## Your and your partners individual contributions
We both did the project seperately so as to adequately learn the coursework, but we collaborated as we went along, discussing the challenges involved. Some portions of our programs are collaborations (random bits such as handling alphabets or commandline argument handling). So we each completed every task required of the assignment, but came together and decided to submit one version for the final submission as per course rules.

## If you have attempted extra credit
Yes.