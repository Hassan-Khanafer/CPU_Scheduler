#include <unistd.h> //This allows to use sysctem calls fork(), wait(), etc
#include <iostream> 
#include <signal.h> 
#include <ctime>
#include <cstdlib>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <stdio.h>
#include <stdlib.h>

using namespace std;
 
int start_time;

int main(int argc, char *argv[]) 
{
    //Retrieves shmid from arguments
    int shmid = atoi(argv[1]);

    //Attach shared memory
    int *random = (int *)shmat(shmid, NULL, 0);

    time_t start_time = time(0);

    while(1){
        usleep(1000000);
        printf("I am Process 2, the random number is: %d\n", *random);

        if(*random == 0){
            cout << "Process 2 is killed" << endl; 
            kill(getpid(), SIGKILL);
            shmdt(random);
        }
    }
    return 0;
}