#include "headers.h"
#include <sys/sem.h>
#include <time.h>
int sem;
union Semun
{
    int val;               		/* value for SETVAL */
    struct semid_ds *buf;  	/* buffer for IPC_STAT & IPC_SET */
    ushort *array;          	/* array for GETALL & SETALL */
    struct seminfo *__buf;  	/* buffer for IPC_INFO */
    void *__pad;
};
void down(int sem)
{
    struct sembuf p_op;

    p_op.sem_num = 0;
    p_op.sem_op = -1;
    p_op.sem_flg = !IPC_NOWAIT;

    if(semop(sem, &p_op, 1) == -1)
    {
        perror("Error in down()");
        exit(-1);
    }
}
void up(int sem)
{
    struct sembuf v_op;

    v_op.sem_num = 0;
    v_op.sem_op = 1;
    v_op.sem_flg = !IPC_NOWAIT;

    if(semop(sem, &v_op, 1) == -1)
    {
        perror("Error in up()");
        exit(-1);
    }
}
void cont(int signum);
void sigusr2(int signum);
void quan(int signum);
/* Modify this file as needed*/
int remainingtime;
key_t  msgqid_sp;
int shmid_process;
int startingclock;
int quantum;
bool x;
int temp;
int stopP=0;
int old;
int finishCLK;
int main(int agrc, char *argv[])
{
    initClk();
    startingclock = getClk();
    temp=startingclock;
    shmid_process = shmget(14000, 5, IPC_CREAT | 0644); //first byte is no of processes , second byte is no of arrived processes rest is processes details
        sem = semget(12345, 1, 0666|IPC_CREAT);
    if(sem==-1)
    {
        perror("Error in create sem  ");
        exit(-1);
    
    }
    union Semun semun;
    semun.val = 1;  	/* initial value of the semaphore, Binary semaphore */
    //printf("\n i am forked and working man \n");
    int*process_quantum = (int *)shmat(shmid_process, (void *)0, 0); //ID of the process
    int remainingtime = *process_quantum;
    signal(SIGCONT,cont);
    double timenow=0;
    //clock_t time= clock();
    old=remainingtime;
    while (remainingtime >0)
     {  
       //printf("\n %f \n",timenow);
        down(sem);
        if(startingclock!=temp)
        {
          remainingtime--;
          startingclock=temp;
          //printf("\n time now is %d remaining time is %d \n",temp,remainingtime);
        }
        temp=getClk();
        up(sem);
    }
    kill(getppid(),SIGUSR1);
    destroyClk(false);    
}

void cont(int signum)
{
x=true;
startingclock=getClk();
temp=startingclock;
remainingtime=old;
}

// void sigusr2(int signum)
// {
// temp=startingclock;
// }