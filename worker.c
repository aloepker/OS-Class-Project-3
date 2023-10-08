#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <signal.h>
#include <sys/msg.h>
#include <sys/types.h>
#include <string.h>
#include <errno.h>

#define SHMKEY 859048
#define BUFF_SZ sizeof ( int )
#define PERMS 0777
typedef struct msgbuffer {
	long mtype;
	char strData[100];
	int intData;
} msgbuffer;


void signal_handler(int signum){
printf("60 Second time limit Reached! Terminating Worker Program\n");
exit(0);
}

int main(int argc, char** argv){
	signal(SIGALRM, signal_handler);
	alarm(3);

	msgbuffer buf;
	buf.mtype = 1;
	int msqid = 0;
	key_t key;
	// aquire key for message queue
	if ((key = ftok("msgq.txt", 1)) == -1) {
		perror("child ftok error");
		exit(1);
	}
	// create message queue
	if ((msqid = msgget(key, PERMS)) == -1) {
		perror("msgget error in child"); // current error is before here
		exit(1);
	}
	printf("Child has access to the message que!\n");

	if(argc>2){
		int shmidc = shmget(SHMKEY, BUFF_SZ, 0777);
		if (shmidc == -1){
			perror("Child process shared memory error!");
			return EXIT_FAILURE;
		}
		int * cint= (int*)(shmat(shmidc,0,0));
		int argSec = atoi(argv[1]);
		int argNano = atoi(argv[2]);
		int timeoutSec = argSec + cint[0];
		int timeoutNano = argNano + cint[1];
		int timeUp = 0;
		int startSec = cint[0];
		int secActive = 0;
		printf("WORKER PID: %d PPID %d SysClockS: %d SysclockNano %d TermTimeS: %d TermTimeNano: %d --Just Starting\n",getpid(),getppid(),cint[0], cint[1], timeoutSec, timeoutNano);
		//loop that checks the clock:
		while(timeUp != 1){
			
			// message queue read test:
			if (msgrcv(msqid, &buf, sizeof(msgbuffer), getpid(), 0) == -1) {
				perror("failed to recieve message form parent");
				exit(1);
			}
			//printing the recieved message test:
			printf("Child %d recieved message: %d\n", getpid(), buf.intData);


			if ( secActive < (cint[0]-startSec) ){
				secActive++;
				printf("WORKER PID: %d PPID %d SysClockS: %d SysclockNano %d TermTimeS: %d TermTimeNano: %d -- %d seconds have passed since starting\n",getpid(),getppid(),cint[0], cint[1], timeoutSec, timeoutNano, secActive);
			}
			if(timeoutSec == cint[0]) {
				if (timeoutNano < cint[1] || timeoutSec < cint[0]) {
					timeUp = 1;
					printf("WORKER PID: %d PPID %d SysClockS: %d SysclockNano %d TermTimeS: %d TermTimeNano: %d --Terminating\n",getpid(),getppid(),cint[0], cint[1], timeoutSec, timeoutNano);
				}
			}
		}
	} else {
		printf("incorrect number of arguments\n");
	}
}
