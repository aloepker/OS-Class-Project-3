#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <signal.h>
#define SHMKEY 859048
#define BUFF_SZ sizeof ( int )
void signal_handler(int signum){
printf("60 SecondTimelimit Reached! Terminating Worker Program\n");
exit(0);
}
int main(int argc, char** argv){
	signal(SIGALRM, signal_handler);
	alarm(3);
	if(argc>2){
		int shmidc = shmget(SHMKEY, BUFF_SZ, 0777);
		if (shmidc == -1){
			printf("Child process shared memory error!");
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
		while(timeUp != 1){
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
