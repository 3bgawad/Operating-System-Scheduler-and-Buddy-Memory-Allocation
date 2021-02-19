#include <stdio.h>
#include <string.h>
#include "headers.h"
#include <stdlib.h>
#include <sys/sem.h>
#include <math.h>
#include <sys/time.h>
#include <sys/sem.h>
int pauseClk;
int lastEntryStart;
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



////CIRCULAR LINKED LIST
int starting = 1;
struct node
{
    int data;
    struct node *next;
};
struct node *head = NULL;
struct node *current = NULL;
struct node *tail = NULL;
struct node *ptrTraverse;

int length()
{
    int length = 1;

    //if list is empty
    if (head == NULL)
    {
        return 0;
    }
    if (head->next == tail)
    {
        return 2;
    }

    current = head->next;

    while (current != head)
    {
        length++;
        current = current->next;
    }

    return length;
}
bool isEmptyCircular()
{
    return (head == NULL);
}

//insert link at the first location
void insert(int data)
{ //create a link
    struct node *newdata = (struct node *)malloc(sizeof(struct node));
    newdata->data = data;
    if (isEmptyCircular())
    {
        starting = 1;
        head = newdata;
        tail = newdata;
        head->next = tail;
        ptrTraverse = head;
        //printf("\n inserted %d \n", head->data);
    }
    else
    {
        //point it to old first node
        newdata->next = tail->next;
        tail->next = newdata;
        tail = newdata;
        //printf("\n inserted %d \n", newdata->data);
    }
}

//delete first item
void deleteNode(int key)
{
    if (head->next == head)
    {
        head = NULL;
        tail = NULL;
        ptrTraverse = NULL;
        return;
    }
    //save reference to first link
    struct node *tempLink = head;
    struct node *tempLink2 = head->next;
    if (head->data == key)
    {
        head = head->next;
        ptrTraverse = head;
        tail->next = head;
        starting = 1;
    }
    while (tempLink2->data != key)
    {
        if (tempLink2 == head)
        {
            return;
        }
        tempLink2 = tempLink2->next;
        tempLink = tempLink->next;
    }
    int g = 0;
    if (ptrTraverse == tempLink2)
    {
        g = 1;
    }
    tempLink->next = tempLink2->next;
    if (g == 1)
    {
        ptrTraverse = tempLink;
    }
    if (tail->data == key)
    {
        tail = tempLink;
    }
    //return the deleted link
}
bool find(int key)
{
    if (head == NULL)
    {
        return false;
    }

    if (head->data == key)
    {
        return true;
    }
    struct node *tempLink = head->next;
    while (tempLink != head)
    {
        if (tempLink->data == key)
        {
            return true;
        }
        tempLink = tempLink->next;
    }
    return false;
}

//display the list
void printList()
{

    struct node *ptr = head;
    //printf("\n[ ");
    if (head->next == head)
    {
        //printf("\n %d \n", head->data);
    }

    //start from the beginning
    if (head != NULL)
    {
        while (ptr->next != head)
        {
            //printf("\n %d \n", ptr->data);
            ptr = ptr->next;
        }
    }

    //printf(" ]\n");
}
//END OF CIRCULAR LINKED LIST

//Function Area
struct nodePCB *roundRobinGetId();
void recieveMessages();
void addProcesstoPCB(Process process);
void forkProcess(int processid, char *argv[]);

struct nodePCB *findMinRemainingTime();
struct nodePCB *findPCB(int id);
struct nodePCB *findMinimumPrio();

void release(struct memory **memory, int starting, int end);
void allocate(int size, int *startmem, int *endmem);

int processid;
void handler(int signum);
void alarmDone(int signum);
void processDone(int signum);
void childStarted(int signum);
void pauseProcess();
int startingTime = 0;
int processarrived = 0;

int first = 1;
int firstime = 0;
int shmid;
int shmid_process;
key_t msgGeneratorScheduler; //the message queue between the scheduler and the process generator
int finished = 0;            //marking finished processes
int finishCLk = 0;
FILE *fptr;
FILE *mem;
int mode;
pid_t processpid;
int processTerminated = 0;
int processFirstTime = 0;
int STRNcondition = 0;
int remainTimeProcess = 0;    //remaining time of a process;
int totalNumberProcesses = 0; //marks the total number of processes

int *process_id;
int *process_quantum;

int arrivedNumber = 0;
pid_t scheduleprocesspid;

bool processRunning;

struct nodePCB *currProcess = NULL;

bool allProcessesArrived = false;

float runningTimeSum = 0;
float WTAsum = 0;
float WTAsumsquared = 0;
float waitingsum = 0;
struct itimerval it_val;
int *memorysizes;
int main(int argc, char *argv[])
{
    shmid = shmget(12000, 4096, IPC_CREAT | 0644);      //first byte is no of processes , second byte is no of arrived processes rest is processes details
    memorysizes= (int *)shmat(shmid, (void *)0, 0);
     int size=11;
	memArray=(struct memory**)malloc(sizeof(struct memory*)*size); //array of pointers with size 11
	for(int i=0;i<size;i++)
	{
		memArray[i]=NULL;
	}
	memArray[10]=(struct memory*)malloc(sizeof(struct memory));
	memArray[10]->next=NULL;
	memArray[10]->starting=0;
	memArray[10]->end=1024-1;
    mem=fopen("memory.log","w");
    fflush(mem);
    fprintf(mem, "At time x allocated y bytes for process z from i to j \n");
    fflush(mem);
    fptr = fopen("scheduler.log", "w");
    fflush(fptr);
    fprintf(fptr, "# At time process y state arr w total z remain y wait k \n");
    fflush(fptr);
    //signal(SIGINT, deletehandler);
    signal(SIGUSR1, processDone);
    signal(SIGALRM, alarmDone);
    //signal(SIGUSR2, childStarted);
    //signal(SIGTERM, processDone);
    shmid_process = shmget(14000, 5, IPC_CREAT | 0644); //first byte is no of processes , second byte is no of arrived processes rest is processes details
    process_quantum = (int *)shmat(shmid_process, (void *)0, 0); //number of processes
    sem = semget(12345, 1, 0666|IPC_CREAT);
    if(sem==-1)
    {
        perror("Error in create sem  ");
        exit(-1);
    
    }
    union Semun semun;
    semun.val = 1;  	/* initial value of the semaphore, Binary semaphore */
    if(semctl(sem, 0, SETVAL, semun) == -1)
    {
        perror("Error in semctl");
        exit(-1);
    }
    //printf("\nselected algo is %s \n", argv[0]);
    msgGeneratorScheduler = msgget(12613, IPC_CREAT | 0644); //message queue between process generator and scheduler
    if (*argv[0] == '1')                                     //HPF sheduler
    {
        //printf("\n HPF is choosen \n");
        mode = 1;
    }
    else if (*argv[0] == '2')
    {
        //printf("\n STRN is choosen \n");
        mode = 2;
    }
    else if (*argv[0] == '3')
    {
        //printf("\n RR is choosen \n");
        mode = 3;
    }
    else
    {
        exit(-1);
        return 0;
    }
    // totalNumberProcesses = __INT16_MAX__; //since i dont know the number of processes the real value will arrive when the dummy variable arrives

    arrivedNumber = 0; //arrival number is 0 at the beginning

    finished = 0; // number of finished processes is 0 at the end

    processRunning = false; //at the beginning no process is running

    allProcessesArrived = false;

    totalNumberProcesses = __INT_MAX__;

    initClk();
    //printf("\n total number of processes is %d \n", totalNumberProcesses);

    while (finished < totalNumberProcesses)
    {
        recieveMessages();
        //printf("\n 5555555555555555555555555555555555555 \n");
    
        if (finished == totalNumberProcesses)
        {
            break;
        }
        if (mode == 1)
        {
            currProcess = findMinimumPrio();
            //printf("\n currProcess id is %d \n",currProcess->processid);
        }
        else if (mode == 2)
        {
            currProcess = findMinRemainingTime();
        }
        else if (mode == 3)
        {
            currProcess = roundRobinGetId();
      
           // printf("\n currProcess id is %d \n",currProcess->processid);
        }


        //printf("\n at line 366: process id scheduled is %d \n", processid);
        int RRquantum = 1;
        if (currProcess->pid == -1) //check if the process hasnt been forked before
        {
            processRunning = true; //to show that a process is running so other process is scheduled
            forkProcess(processid, argv);
            if (first == 1)
            {
                firstime = currProcess->arrival;
                first = 0;
            }
            if (mode == 3)
            {
                it_val.it_value.tv_sec =RRquantum;
                it_val.it_value.tv_usec =50000;
                it_val.it_interval.tv_sec = 0;
                it_val.it_interval.tv_usec=0;
                if (setitimer(ITIMER_REAL, &it_val, NULL) == -1)
                {
                    perror("error calling setitimer()");
                    exit(1);
                }
                //alarm(RRquantum); //round robin quantum
            }
            
                
            startingTime = getClk();                                 //starting time of process
            currProcess->wait = startingTime - currProcess->arrival; //set the wait as response time
            fflush(fptr);
            fprintf(fptr, "At time %d process %d started arr %d total %d remain %d wait %d \n", startingTime, currProcess->processid, currProcess->arrival, currProcess->runtime, currProcess->remainingtime, currProcess->wait);
            fflush(fptr);
            currProcess->state = 1; //the process is running
        }
        else
        {
            processRunning = true; //to show that a process is running so other process is scheduled
            if (mode == 3)
            {
                it_val.it_value.tv_sec =RRquantum;
                it_val.it_value.tv_usec =50000;
                it_val.it_interval.tv_sec = 0;
                it_val.it_interval.tv_usec=0;
                if (setitimer(ITIMER_REAL, &it_val, NULL) == -1)
                {
                    perror("error calling setitimer()");
                    exit(1);
                }
                kill(currProcess->pid,SIGCONT);
                //alarm(RRquantum); //starting time of process
            }
            else
            {               
             kill(currProcess->pid, SIGCONT);
            }
            
            startingTime = getClk();
            currProcess->wait += startingTime - currProcess->lastexectime;
            fflush(fptr);
            fprintf(fptr, "At time %d process %d resumed arr %d total %d remain %d wait %d\n", startingTime, currProcess->processid, currProcess->arrival, currProcess->runtime, currProcess->remainingtime, currProcess->wait);
            fflush(fptr);
            currProcess->state = 1; //the process is running
        }
        //printf("\n i finished a process with finished number = %d and finished clock = %d \n", finished, getClk());
    }
    FILE *pref;
    pref = fopen("scheduler.pref", "w");

    for (int i = 0; i < totalNumberProcesses; i++)
    {
    }
    //printf(" \nlast time is %d \n ", finishCLk);
    fprintf(pref, "CPU utilization = %f \n", runningTimeSum/(float)((finishCLk - firstime) ) * 100.0);
    fprintf(pref, "Avg WTA = %.2f\n", ((WTAsum / (totalNumberProcesses))));
    fprintf(pref, "Avg Waiting = %.2f\n", (waitingsum / (totalNumberProcesses)));
    float avg = WTAsum / (totalNumberProcesses);
    float avg2 = WTAsumsquared / (totalNumberProcesses);
    float avg22 = avg * avg;
    float variance = avg2 - avg22;
    fprintf(pref, "Std WTA = %.2f\n", sqrt(variance));
    //printf(" \n all processes finished \n");
    fclose(fptr);
    fclose(pref);
    destroyClk(false);
    shmctl(shmid_process, IPC_RMID, (struct shmid_ds *)0);
    exit(shmid_process);
}

//////////f///////////////////////////FUnctions Area////////////////////////

struct nodePCB *findMinimumPrio()
{
    if (headPCB == NULL)
    {
        return NULL;
    }
    else if (headPCB->next == NULL)
    {
        return headPCB;
    }
    struct nodePCB *traverseptr = headPCB;
    struct nodePCB *minPrio = headPCB;
    int min = __INT_MAX__;
    while (traverseptr != NULL)
    {
        if (traverseptr->priority < min)
        {
            min = traverseptr->priority;
            minPrio = traverseptr;
        }
        traverseptr = traverseptr->next;
    }
    return minPrio;
}
struct nodePCB *findMinRemainingTime()
{
    if (headPCB == NULL)
    {
        return NULL;
    }
    else if (headPCB->next == NULL)
    {
        return headPCB;
    }
    struct nodePCB *traverseptr = headPCB;
    struct nodePCB *minRemaining = headPCB;
    int min = __INT_MAX__;
    while (traverseptr != NULL)
    {
        if (traverseptr->remainingtime < min)
        {
            min = traverseptr->remainingtime;
            minRemaining = traverseptr;
        }
        traverseptr = traverseptr->next;
    }
    return minRemaining;
}

struct nodePCB *findPCB(int id)
{
    if (headPCB == NULL)
    {
        return NULL;
    }
    else if (headPCB->next == NULL)
    {
        return headPCB;
    }
    struct nodePCB *traverseptr = headPCB;
    while (traverseptr != NULL)
    {
        if (traverseptr->processid == id)
        {
            return traverseptr;   
        }
        else
        {
            traverseptr=traverseptr->next;
        }

    }
    return NULL;
}

struct nodePCB *roundRobinGetId()
{
    if (starting == 1)
    {
        ptrTraverse = head;
        starting = 2;
    }
    else
    {
        //printList();
        ptrTraverse = ((ptrTraverse)->next);
    }
    
    processid = ((ptrTraverse)->data);
    //printf("\n process id is %d \n",processid);
    return findPCB(processid);
}
void recieveMessages() //called in case of processes arrives, now if we call this function we are waiting for a process to come
{                      // then it gets all processes that arrived at the same time and updates the Queue
    while (1)
    {
        int rec_val;
        Process process;
        //printf("\n waiting for process \n");
        rec_val = msgrcv(msgGeneratorScheduler, &process, sizeof(Process), 0, !IPC_NOWAIT); //sleep in case nothing arrives and wait for a signal to come to schedule another process
        if (rec_val == -1)
        {
            //printf("\n in recieve messages: a signal arrived so get out and continue scheduling \n");
            if(headPCB!=NULL || finished==totalNumberProcesses)
            {
            break;
            }

        }
        else if (process.idNum == 1000) //dummy variable for
        {
            //printf("\n dummy variable arrived bro \n");
            if (processRunning == false) //if a process is not running outside schedule a new process else continue sleeping and recieving
            {
                //printf("\n i am breaking \n");
                return;
            }
        }
        else if (process.idNum == 2000)
        {
            totalNumberProcesses = arrivedNumber;
            allProcessesArrived = true;
            //printf("\n i the scheduler recieved all processes info \n");
            if (processRunning == false) // in case all arrived at the same time
            {
                break;
            }
        }
        else
        {
            if (processRunning == true && process.runtimeNum < currProcess->remainingtime - (getClk() - startingTime) && mode == 2) //there is a process with a less remaining time
            {
                if (currProcess->state == 1) //if the process state is running
                {
                    pauseProcess();
                }
            }
            addProcesstoPCB(process);
        }
    }
    //printf("\n here \n");
}
void addProcesstoPCB(Process process)
{
    struct nodePCB *newdata = (struct nodePCB *)malloc(sizeof(struct nodePCB));
    newdata->processid = process.idNum;
    newdata->priority = process.priorityNum;
    newdata->runtime = process.runtimeNum;
    newdata->remainingtime = process.runtimeNum;
    newdata->arrival = process.arrivalTime;
    newdata->state = 0;
    newdata->lastexectime = 0;
    newdata->wait = 0;
    newdata->pid = -1;
    newdata->memsize=memorysizes[process.idNum-1];
    int startMem, endMemo = 0;
    allocate(newdata->memsize,&startMem,&endMemo);
    newdata->startingmem=startMem;
    newdata->endmem=endMemo;
    fflush(mem);
    fprintf(mem,"At time %d allocated %d bytes for process %d from %d to %d \n",process.arrivalTime,newdata->memsize,newdata->processid,newdata->startingmem,newdata->endmem);
    fflush(mem);
    runningTimeSum+=newdata->runtime;
    insertPCB(newdata);
    //printf("\n id %d prio %d runtime %d at arrival time %d with memory %d \n", process.idNum, process.priorityNum, process.runtimeNum, process.arrivalTime,newdata->memsize);
    if (mode == 3) //ROund Robin
    {
        insert(process.idNum); //to add the index
    }
    arrivedNumber++;
}
void forkProcess(int processid, char *argv[])
{
    *process_quantum = currProcess->runtime; //total runtime
    scheduleprocesspid = fork();
    if (scheduleprocesspid == 0) //run the process with the remaining time as quantum FCFS
    {
        execvp("./process.out", argv);
    }
    else if (scheduleprocesspid == -1)
    {
        //printf("There is an error while calling fork()");
    }
    else
    {
        currProcess->pid = scheduleprocesspid; //set the process pid so we dont fork again
    }
}
// Handlers area
void processDone(int signum)
{
    finishCLk = getClk();
    release(memArray,currProcess->startingmem,currProcess->endmem);
    currProcess->remainingtime = 0; //UPDATE REAMIN TIME
    int TA = (finishCLk - currProcess->arrival);
    float WTA = ((float)(finishCLk - currProcess->arrival)) / currProcess->runtime;
    waitingsum += currProcess->wait;
    WTAsum += WTA;
    WTAsumsquared += (WTA * WTA);
    finished++;
    //printf("\nnumber of finished is %d and finished clock is %d and the id is %d \n ", finished, finishCLk, processid + 1);
    fflush(mem);
    fprintf(mem,"At time %d freed %d bytes from process %d from %d to %d \n",finishCLk,currProcess->memsize,currProcess->processid,currProcess->startingmem,currProcess->endmem);
    fflush(mem);
    fflush(fptr);
    fprintf(fptr, "At time %d process %d finished arr %d total %d remain %d wait %d TA %d WTA %.2f \n", finishCLk, currProcess->processid, currProcess->arrival, currProcess->runtime, currProcess->remainingtime, currProcess->wait, TA, WTA);
    fflush(fptr);
    if (mode == 3)
    {
                it_val.it_value.tv_sec =0;
                it_val.it_value.tv_usec =0;
                it_val.it_interval.tv_sec = 0;
                it_val.it_interval.tv_usec=0;
        if (setitimer(ITIMER_REAL, &it_val, NULL) == -1)
        {
            perror("error calling setitimer()");
            exit(1);
        }
        //alarm(0);
        deleteNode((ptrTraverse)->data); //delete the element that finished
    }
    deletePCB(currProcess->processid); // delete the PCB after finish
    processRunning = false;            //to allow other processes to work
    signal(SIGUSR1, processDone);
}
void alarmDone(int signum)
{
    //printf("\n time is over \n");
         //get the pausing time
    pauseClk = getClk(); 
    pauseProcess();
}

void pauseProcess()
{
    down(sem);
    kill(currProcess->pid, SIGSTOP); //stop current process
    up(sem);
    if(mode==2)
    {
        pauseClk = getClk(); 
    
    //printf("\n quantum has finished of process %d at time %d \n", proscessid, pauseClk);
    currProcess->lastexectime = pauseClk;
    currProcess->remainingtime = currProcess->remainingtime-(pauseClk-startingTime);
    // if(currProcess->remainingtime<=0)
    // {
    //     kill(getpid(),SIGUSR1);
    //     return;
    // }
    fflush(fptr);
    fprintf(fptr, "At time %d process %d stopped  arr %d total %d remain %d wait %d\n",pauseClk, currProcess->processid, currProcess->arrival, currProcess->runtime, currProcess->remainingtime, currProcess->wait);
    fflush(fptr);
    printf("\nAt time %d process %d stopped  arr %d total %d remain %d wait %d\n",pauseClk, currProcess->processid, currProcess->arrival, currProcess->runtime, currProcess->remainingtime, currProcess->wait);

    processRunning = false;
    currProcess->state = 0; //back to ready
}

void allocate(int size, int *startA, int *endA)
{
    float index = log((double)size) / log(2.0);
    if (index != (int)index) //it is a float then round
    {
        index = (int)(index + 1);
    }
    if (memArray[(int)index] == NULL) //there are no free spaces in the desired memory
    {
        int finishindex = -1;
        for (int i = index + 1; i <= 10; i++) //backtrace to divide memory
        {
            if (memArray[i] == NULL)
            {
                continue;
            }
            else
            {
                finishindex = i;
                break;
            }
        }
        if (finishindex == -1)
        {
            return; //we cannot allocate this memory
        }
        for (int i = finishindex; i > index; i--)
        {
            int startindex = memArray[i]->starting;
            int endindex = memArray[i]->end;
            int medianindex = (startindex + (endindex + 1)) / 2;
            int start1new = startindex;
            int end1new = medianindex - 1;
            int start2new = medianindex;
            int end2new = endindex;
            insertmem(memArray, i - 1, start1new, end1new);
            insertmem(memArray, i - 1, start2new, end2new);
            deleteHead(memArray, i); //delete the head;
        }
    }
    *startA = memArray[(int)index]->starting;
    *endA = memArray[(int)index]->end;
    deleteHead(memArray, index);
}

void release(struct memory **memory, int starting, int end)
{
    int index = log((double)(end - starting + 1)) / log(2.0);
    if (index == 10) //breaking condition no more merging after that
    {
        insertmem(memory, 10, starting, end);
        return;
    }
    struct memory *traverse = memory[index];
    float check = (float)starting / (pow(2, index + 1));
    bool condition = (float)starting / (pow(2, index + 1)) == (int)(starting / (pow(2, index + 1))); //if true it is starting point , else it is end point
    //checking for merging
    while (traverse != NULL)
    {
        if (condition == true)
        {
            if (end == (traverse->starting - 1)) //we found a merging
            {
                int en = traverse->end;
                deleteMemory(memory, index, traverse->starting, traverse->end);
                release(memory, starting, en); //merge blocks
                return;
            }
        }
        else
        {
            if (traverse->end + 1 == starting)
            {
                int st = traverse->starting;
                deleteMemory(memory, index, traverse->starting, traverse->end);
                release(memory, st, end);
                return;
            }
        }
        traverse = traverse->next;
    }
    //there are no merging points just insert the
    insertmem(memory, index, starting, end);
}
