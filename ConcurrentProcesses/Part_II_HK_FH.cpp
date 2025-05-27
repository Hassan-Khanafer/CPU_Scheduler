#include <unistd.h> //This allows to use sysctem calls fork(), wait(), etc
#include <iostream> 
#include <signal.h> 
#include <ctime>
#include <cstdlib>
#include <sys/wait.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>

using namespace std;
 
int start_time;
bool flag9;

int temp1;
int temp2;

union semun {
    int val;               // Value for SETVAL
    struct semid_ds *buf;  // Buffer for IPC_STAT, IPC_SET
    unsigned short *array; // Array for GETALL, SETALL
} arg;

int main() 
{ 
    time_t start_time = time(0);
    int shmid = shmget(IPC_PRIVATE , sizeof(int), IPC_CREAT | 0666);

    int *random = (int *)shmat(shmid, NULL, 0); 

    int semid = semget(IPC_PRIVATE , 1, IPC_CREAT | 0666);

    union semun sem_union;
    sem_union.val = 1;
    semctl(semid, 0, SETVAL, sem_union);

    srand(time(NULL));

    pid_t c_pid = fork(); 

    while(1){
        if (c_pid == -1) { 
            perror("fork"); 
            exit(EXIT_FAILURE); 
        } 
        else if (c_pid > 0) { 
            usleep(1000000);
            
            struct sembuf sem_op;
            sem_op.sem_num = 0;
            sem_op.sem_op = -1; // Wait (P operation)
            sem_op.sem_flg = 0;
            semop(semid, &sem_op, 1); 

            *random = rand() % 11;
            temp1 = *random;

            sem_op.sem_op = 1; // Signal (V operation)
            semop(semid, &sem_op, 1); 

            cout << "I am Process 1, the random number is: " << temp1 << endl; 

            if (temp1 == 9){
                flag9 = true;
            }
            if(temp1 == 0 && flag9){
                wait(NULL);
                shmdt(random);
                semctl(semid, 0, IPC_RMID, 0);
                shmctl(shmid, IPC_RMID, NULL);
                cout << "Process 1 is killed" << endl; 
                kill(getpid(), SIGKILL);
            }

            
        }
        else if (c_pid == 0) { 
            struct sembuf sem_op;
            sem_op.sem_num = 0;
            sem_op.sem_op = -1; // Wait (P operation)
            sem_op.sem_flg = 0;
            semop(semid, &sem_op, 1); // Lock semaphore

            temp2 = *random;

            sem_op.sem_op = 1; // Signal (V operation)
            semop(semid, &sem_op, 1); // Unlock semaphore

            if(temp2 == 9){
                char shmid_str[10];
            sprintf(shmid_str, "%d", shmid);

            execlp("./Part_II_HK_FH_program2.exe", "Part_II_HK_FH_program2.exe", shmid_str, (char *)NULL);
            }
            
        }
    }
    return 0; 
} 
