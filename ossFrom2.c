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
// Almost!  pass random numbers to the worker
//   process table should indicate if each slot is empty or not (initialize all spots as empty,, based on total number of processes maybe)
//   oss should update the process table after each fork
//   output process table every half "second" (pdf has pseudocode)
int sysClockNano = 0;
int sysClockSec = 0;
void incrementClock(){
	sysClockNano = sysClockNano + 2000; // professor suggests around 10mil incrementation at a time, we will see
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
void help(){
	printf("The options for the program are:\n");
	printf("-n <number>   this sets the number of processes to launch\n");
	printf("-s <number>   this sets how many processes can run at once\n");
	printf("-t <number>   this sets the maximum time in seconds a random number generator can chose from for the lifespan of the worker processes\n");
	printf("example:\n./oss -n 3 -s 2 -t 3\n");
}
#define SHMKEY 859048
#define BUFF_SZ sizeof (int)
struct PCB {
	int occupied;        //either true or false
	pid_t pid;	     //process id of this child
	int startSeconds;    //time when it was forked
	int startNano;       //time when it was forked
};
struct PCB processTable[20];
int main(int argc, char** argv){
	int option, status;
	int numWorkers = 0;
	int workerLimit = 0;
	int timeLimit = 0;
	int prevSec = 0;
	while((option = getopt(argc, argv, "hn:s:t:")) != -1){
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
	//initializing shared memor
	int shmid = shmget ( SHMKEY, BUFF_SZ, 0777 | IPC_CREAT );
	if ( shmid == -1 ){
		perror("Shared Memory Creation Error!!!");
		return EXIT_FAILURE;
	}
	char * paddr = ( char * )( shmat ( shmid, 0, 0 ) );
	int * shmTime = ( int * )( paddr );
	shmTime[0]=0;
	shmTime[1]=0;
	//that is it for starting shared memory
 	shmdt(shmTime);
	printf("Number of workers Selected:%d\nNumber of Workers at a time:%d\nNumber of loops for each Worker:%d\n", numWorkers, workerLimit, timeLimit);
	int i=0,j=0;
	//fork calls:
	pid_t childPid;
	int statPid;
//verify making this a while loop will work 
//	while (i<numWorkers){

	for(i=0;i<numWorkers;i++){


printf("\nwait if ahead:\n");
		if(i >= workerLimit) {
// this wait breaks my simultanous limit, need to consider logic around this problem
//printf("\npre worker limit wait\n");
			statPid = waitpid(-1, &status, WNOHANG);
//			wait(NULL);
//printf("\npost worker limit wait\n");
		}
		childPid = fork();
		if (childPid == -1){
			printf("Fork Process Failed!\n");
			return EXIT_FAILURE;
		}
	//child side of the fork if, why does random seem to keep giving the same number for seconds??
		if (childPid == 0) {
			int timeSec = randSeconds(timeLimit);
			int timeNano = randNano();
			char secArg[10];
			char nanoArg[10];
			sprintf(secArg, "%d", timeSec);
			sprintf(nanoArg, "%d", timeNano);
			char * args[] = {"./worker", secArg, nanoArg, NULL};
//			processTable[i].occupied = 1;
//			processTable[i].pid = getpid();
//			processTable[i].startSeconds = sysClockSec;
//			processTable[i].startNano = sysClockNano;
			execvp("./worker", args);
		}
	}//  for loop
	//parent side of fork if
	if(childPid != 0) {
	int statusPid;
	//parent shared memory
		int shmidp = shmget(SHMKEY, BUFF_SZ, 0777);
		if (shmidp == -1){
			printf("Parent process shared memory error!\n");
			return EXIT_FAILURE;
		}
		int * ppint = (int*)(shmat(shmidp,0,0));
		int nanoFlag = 0;
			while (j < numWorkers) {
				statusPid = waitpid(-1, &status, WNOHANG);
				if (statusPid != 0 ){
					printf("A Child Process completed successfully!\n");
					j++;
//update pcb here regarding terminated child

//					printf("Shared memory clock contains the following: Seconds: %d and Nanoseconds: %d\n",ppint[0],ppint[1]);
				}
				incrementClock();
				ppint[0] = sysClockSec;
				ppint[1] = sysClockNano;
		
				if (ppint[1] > 500000000 && nanoFlag == 0){
					//print and set nano flag
					nanoFlag = 1;
	//pcb table prints here:
	printf("OSS PID:%d SysClockS: %d SysclockNano: %d\n",getpid(), ppint[0], ppint[1]);
	printf("Process Table:\n");
	printf("Entry Occupied PID   StartS StartN\n");
	int k;
	for (k=0;k<20;k++){
		printf("  %d     %d      %d     %d      %d\n", k, processTable[k].occupied, processTable[k].pid, processTable[k].startSeconds, processTable[k].startNano);
	}
				}else if(ppint[0]>prevSec){
					nanoFlag = 0;
					prevSec = ppint[0];
	//pcb table prints here
	printf("OSS PID:%d SysClockS: %d SysclockNano: %d\n",getpid(), ppint[0], ppint[1]);
	printf("Process Table:\n");
	printf("Entry Occupied PID   StartS StartN\n");
	int k;
	for (k=0;k<20;k++){
		printf("  %d     %d      %d     %d      %d\n", k, processTable[k].occupied, processTable[k].pid, processTable[k].startSeconds, processTable[k].startNano);
	}
				}
			}
//end of while above
//	i++;
//	}
	shmdt(ppint);
	shmctl(shmid, IPC_RMID, NULL);
printf("shared memory shutdown successful\n");
	}
}
