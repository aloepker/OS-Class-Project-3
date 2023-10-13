#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <string.h>
#include <time.h>
#include <sys/msg.h>
#include <errno.h>

int sysClockNano = 0;
int sysClockSec = 0;
void incrementClock(){
	sysClockNano = sysClockNano + 100000000;
	if (sysClockNano > 1000000000){
		sysClockSec++;
		sysClockNano = sysClockNano - 1000000000;
	}
}
int randSeconds(int max){
	srand(time(NULL)+1);
	return rand()%max;
}
int randNano(){
	srand(time(NULL));
	return 1+rand()%1000000000;
}
struct PCB {
	int occupied;
	pid_t pid;
	int startSeconds;
	int startNano;
};
struct PCB processTable[20];

void printPCB(int smS, int smN){
	printf("OSS PID: %d SysClockS: %d SysClockNano: %d\n", getpid(), smS, smN);
	printf("Process Table:\n");
	printf("Entry Occupied PID   StartS StartN\n");
	int m;
	for (m=0;m<20;m++){
		printf("  %d     %d      %d     %d      %d\n", m, processTable[m].occupied, processTable[m].pid, processTable[m].startSeconds, processTable[m].startNano);
	}
}
void help(){
	printf("The options for the program are:\n");
	printf("-n <number>   this sets the number of processes to launch\n");
	printf("-s <number>   this sets how many processes can run at once\n");
	printf("-t <number>   this sets the maximum time in seconds a random number generator can chose from for the lifespan of the worker processes\n");
	printf("-f <\"output_file_name.txt\">   this sets the name of the output file\n");
	printf("example:\n./oss -n 3 -s 2 -t 3 -f \"output.txt\"\n");
}
#define SHMKEY 859048
#define BUFF_SZ sizeof (int)

#define PERMS 0777
typedef struct msgbuffer {
	long mtype;
	char strData[100];
	int intData;
} msgbuffer;

int main(int argc, char** argv){
	int option, status;
	int numWorkers = 0;
	int workerLimit = 0;
	int timeLimit = 0;
	int prevSec = 0;
	char *logFile= "log_file.txt";
	while((option = getopt(argc, argv, "hn:s:t:f:")) != -1){
		switch(option){
			case 'h':
				help();
				return EXIT_SUCCESS;
			case 'n':
				numWorkers = atoi(optarg);
				break;
			case 's':
				workerLimit = atoi(optarg);
				break;
			case 't':
				timeLimit = atoi(optarg);
				break;
			case 'f':
				logFile = optarg;
				break;
			case '?':
				if ((optopt = 'c')){
					printf("Option %c requires an argument\n", optopt);
				} else if (isprint(optopt)){
					printf("Unknown Character '\\x%x'.\n", optopt);
				}
				return 1;
			default:
				help();
				return EXIT_SUCCESS;
		}
	}
	//open output file
	FILE *outputFile = fopen(logFile, "w");
	if (outputFile==NULL){
		printf("Error opening output file!\nTerminating program\n");
		return EXIT_FAILURE;
	}
	//initializing shared memory
	int shmid = shmget ( SHMKEY, BUFF_SZ, 0777 | IPC_CREAT );
	if ( shmid == -1 ){
		perror("Shared Memory Creation Error!!!\n");
		return EXIT_FAILURE;
	}
	char * paddr = ( char * )( shmat ( shmid, 0, 0 ) );
	int * shmTime = ( int * )( paddr );
	shmTime[0]=123;
	shmTime[1]=123;
 //	shmdt(shmTime);
	//message que initial implementation:
	msgbuffer buf0, buf1;
	int msqid;
	key_t key;
	system("touch msgq.txt");
	//set message queue key
	if ((key = ftok("msgq.txt", 1)) == -1) {
		perror("parent ftok error");
		exit(1);
	}
	//creates message queue
	if ((msqid = msgget(key, PERMS | IPC_CREAT)) == -1) {
		perror("msgget in parent error");
		exit(1);
	}
	//print user input verification:
	printf("OSS: Number of workers Selected: %d\nNumber of Workers at a time: %d\nNumber of loops for each Worker: %d\nOutput file: %s\n", numWorkers, workerLimit, timeLimit,logFile);

	int i=0,j=0;
	//fork calls:
	pid_t childPid;
	int statPid;
	msgbuffer rcvbuf;
	int worker = 1;
	
	//oss while loop:
//while workers still active:
	while (i<numWorkers){
// in this loop:
//if shoud create a worker, create a worker:  if not @ worker limit also if not at simultaneous limit, create worker and update pcb

//increment and update clock

//if half a second has passed, print the pcb

//send and recieve messages to/from workers: loop through pcb one at a time. cycle method to next active worker, then send message (complex logic)

//if worker terminates, update pcb



//first up:
//if can create worker, create worker: if not @ worker limit also if not @ simultaneous limit, create worker and update pcb
		childPid = fork();
		if (childPid == -1){
			printf("Fork Process Failed!\n");
			return EXIT_FAILURE;
		}
		//child side of the fork if
		if (childPid == 0) {
			int timeSec = randSeconds(timeLimit);
			int timeNano = randNano();
			char secArg[10];
			char nanoArg[10];
			sprintf(secArg, "%d", timeSec);
			sprintf(nanoArg, "%d", timeNano);
			char * args[] = {"./worker", secArg, nanoArg, NULL};
			execvp("./worker", args);
		}
//			processTable[i].occupied = 1;
//			processTable[i].pid = getpid();
//			processTable[i].startSeconds = sysClockSec;
//			processTable[i].startNano = sysClockNano;
		//parent side of fork if
//end of launch if, maybe. if child is launched, pcb will change. so maybe check to see if pcb has changed from previous, but might not need to
		if(childPid != 0) {
			int statusPid;

//increment and update clock
			incrementClock();
int nanoFlag = 0;//? should this be set now? I think this needs to be before the  bit while loop!!!
			//update clock into shared memory
				shmTime[0] = sysClockSec;//was ppint
				shmTime[1] = sysClockNano;//was ppint


//if half a second has passed, print the pcb
				//time check
				if(shmTime[1] > 500000000 && nanoFlag == 0){
					nanoFlag = 1;
					printPCB(shmTime[0], shmTime[1]);
				}else if(shmTime[0] > prevSec){
					nanoFlag = 0;
					printPCB(shmTime[0], shmTime[1]);
				}

//send and recieve messages to/from workers: loop through pcb one at a time. cycle method to next active worker, then send message (complex logic)

		//now lets try adding a message for the que here to start
		printf("parent while before message send:\n");		
			buf1.mtype = childPid;
			buf1.intData = childPid;
			strcpy(buf1.strData, "Message to Child form Parent");
			if ((msgsnd(msqid, &buf1, sizeof(msgbuffer)-sizeof(long), 0)) == -1) {
		////// ERROR message is pointing here!!!!        !!!!!!!!!!!!!!!!!!!!!!!!!                     
				perror("msgsnd to child failed");
				exit(1);
			}
	//		int messageArrived = 0;
	//		while (messageArrived == 0){
	printf("parent while before message rcv:\n");		

			if (msgrcv(msqid, &rcvbuf, sizeof(msgbuffer), getpid(), 0) == -1){
				perror("failed to recieve message in parent\n");
				exit(1);
			}
			printf("OSS: Parent %d recieved message: %d\n", getpid(), rcvbuf.intData);

	//old non blocking wait		statusPid = waitpid(-1, &status, WNOHANG);
	//				if (statusPid != 0 ){
//blocking wait currently implemented!
					wait(&statusPid);// so once a child is launched, I get stuck here intil it self destructs, then I pass this code wall.
	//needs to be a non blocking wait to be able to keep incremeting the clock as well as continue to fork children in a timley manner

					printf("OSS: A Child Process completed successfully!\n");// rm later, this is for my debugging
				//	fprintf(outputFile,"Shared memory clock contains the following: Seconds: %d and Nanoseconds: %d\n",shmTime[0],shmTime[1]);
		i++;
		//printf("end of parent while loop after iterating i:\n");
	}


		//close shared memory and output file:
//maybe set  if loop condition wil be met, run this code
		shmdt(shmTime);//was ppint
		shmctl(shmid, IPC_RMID, NULL);
		printf("OSS: shared memory shutdown successful\n");
		fclose(outputFile);
		//also clear message ques:
		if (msgctl(msqid, IPC_RMID, NULL) == -1){
			perror("msgctl to get rid of que in parent ");
			exit(1);
		}
	}//end of parent side of fork if
}
