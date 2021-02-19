#include "headers.h"
#include <string.h>
#include <sys/sem.h>


void clearResources(int);

int *readFile(int *length)
{
    FILE *fp;
    char *line = NULL;
    size_t len = 0;
    int read;
    int size = 0;

    fp = fopen("processes.txt", "r");
    if (fp == NULL)
        exit(EXIT_FAILURE);
    while ((read = getline(&line, &len, fp)) != -1)
    {
        size = size + 1;
    }
    *length = size - 1;
    int *arr = malloc((size - 1) * sizeof(int) * 5);
    line = NULL;
    len = 0;

    fp = fopen("processes.txt", "r");
    int count = 0;
    while ((read = getline(&line, &len, fp)) != -1)
    {
        if (line[0] == '#')
        {
            continue;
        }
        int idint, arrivalint, runtimeint, prioint,memsize;
        char *id = strtok(line, "\t ");
        sscanf(id, "%d", &idint);
        char *arrival = strtok(NULL, "\t");
        sscanf(arrival, "%d", &arrivalint);
        char *runtime = strtok(NULL, "\t ");
        sscanf(runtime, "%d", &runtimeint);
        char *prio = strtok(NULL, "\t");
        sscanf(prio, "%d", &prioint);
        char *mem = strtok(NULL, "\t");
        sscanf(mem, "%d", &memsize);
        arr[count] = idint;          // id
        arr[count + 1] = arrivalint; //arrival time
        arr[count + 2] = runtimeint; //runtime
        arr[count + 3] = prioint;    //prioirty
        arr[count+4]=memsize;
        count += 5;
        //count+=4;
    }

    fclose(fp);
    return arr;
}
/******Resources ****/
key_t msgGeneratorScheduler;
pid_t scheduler;
int shmid;
int main(int argc, char *argv[])
{
    shmid = shmget(12000, 4096, IPC_CREAT | 0644);      //first byte is no of processes , second byte is no of arrived processes rest is processes details
    pid_t pid;
    pid_t ppid = getppid();
    signal(SIGINT, clearResources);

    // TODO Initialization
    // 1. Read the input files.
    int noprocesses = 0;

    int *processes = readFile(&noprocesses); //returns an array with all the info about the program
    //printf("\n size is %d", noprocesses);
    int count = 0;
    ////SENDING ARRIVAL TIMES TO THE SCHEDULER

    // 2. Ask the user for the chosen scheduling algorithm and its parameters, if there are any.
    char algo[1];
    printf("\nSelect the scheduling algorithm 1-HPF 2-STRN 3-RR\n");
    gets(algo);
    // 3. Initiate and create the scheduler and clock processes.
    //pid = ppid;
    pid = fork();
    if (pid == 0)
    {
        execvp("./clk.out", argv);
    }
    else if (pid == -1)
    {
        printf("There is an error while calling fork()");
    }
    else
    {
        initClk();
        scheduler = fork();
        if (scheduler == 0)
        {
            char *arg[4];
            arg[0] = algo;
            execvp("./scheduler.out", arg);
        }
        else if (scheduler == -1)
        {
            //printf("There is an error while calling fork()");
        }

        else
        {
            // 4. Use this function after creating the clock process to initialize clock
            // To get time use this
            // TODO Generation Main Loop
            // 5. Create a data structure for processes and provide it with its parameters.
            int* memorysizes= (int *)shmat(shmid, (void *)0, 0);
            struct Queue *queue = createQueue(noprocesses);
            int count=0;
            for (int i = 0; i < noprocesses *5 ; i += 5)
            {
                enqueue(queue, processes[i + 1]); //runtime
                memorysizes[count]=processes[i+4];
                //printf("\n memory sizes = %d \n",memorysizes[count]);
                //printf("\n arrival times are %d\n",processes[i+1]);
                count++;
            }
            int idindex = 0;
            msgGeneratorScheduler = msgget(12613, IPC_CREAT | 0644); //message queue between process generator and scheduler to make him work one the first procese(s) arrives
            int arrived = 0;
            int first = 1;

        
            while (!isEmpty(queue))
            {
                int x = getClk();

                if (front(queue) == x) // a process arrived
                {
                    int countArrived = 0;
                    Process process;

                    while (front(queue) == x) //if many processes arrived at the same time
                    {
                   // Process process;
                    //printf("\nprocess %d arrived at arrival time %d at clk time %d\n", processes[idindex], front(queue), x);
                    process.idNum = processes[idindex]; //send process id
                    process.arrivalTime = processes[idindex + 1];
                    process.runtimeNum = processes[idindex + 2];  //send process runtime
                    process.priorityNum = processes[idindex + 3]; //send process priority
                    //process.memory=processes[idindex+4];
                    //idindex += 5;
                    idindex+=5;
                    dequeue(queue);

                    int send_val;
                    send_val = msgsnd(msgGeneratorScheduler, &process, sizeof(Process), IPC_NOWAIT);
                    if(send_val == -1)
                    {
                          perror("Errror in sending last dummy");
                    
                    }

                }

                    if (isEmpty(queue)) //dummy variable if the last process arrived
                    {
                       // Process dummy;
                        process.idNum = 2000; //idnumber dummy -20
                        process.arrivalTime = 0;
                        process.runtimeNum = 0;
                        process.priorityNum = 0;
                        //process.memory=12;
                        int send_val;
                        send_val = msgsnd(msgGeneratorScheduler, &process, sizeof(Process), IPC_NOWAIT);
                       // printf("\n seding last dummy \n");
                        if (send_val == -1)
                        {
                            perror("Errror in sending last dummy");
                        }
                        break;
                    }
                    else //dummy variable after each batch arrives
                    {
                        //Process dummy;
                        process.idNum = 1000; //idnumber dummy -1
                        process.arrivalTime = 0;
                        process.runtimeNum = 0;
                        process.priorityNum = 0;
                        //process.memory=12;
                        int send_val;
                        send_val = msgsnd(msgGeneratorScheduler, &process, sizeof(Process), IPC_NOWAIT);
                        if (send_val == -1)
                        {
                            perror("Errror in sending dummy");
                        }
                    }
                }
                //Here the process generated sent all the processes at that runtime now we send the dummy value
            }
            // 6. Send the information to the scheduler at the appropriate time.
            // 7. Clear clock resources
            printf("\nall processes arrived\n");
            //destroyClk(true);
            int stat_lock;
            wait(&stat_lock);
            destroyClk(true);
        }
    }
}

void clearResources(int signum) //deleting the memory
{
    //destroyClk(true)
    msgctl(msgGeneratorScheduler, IPC_RMID, (struct msqid_ds *)0);
    printf("\n shared memory is deleted \n");
    exit(0);
}
