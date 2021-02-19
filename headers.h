#include <stdio.h>      //if you don't use scanf/printf change this include
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/file.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <sys/msg.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <string.h>
#include<math.h>

typedef short bool;
#define true 1
#define false 0

#define SHKEY 300

typedef struct Process Process;
struct Process
{
    short idNum;
    int priorityNum;
    short runtimeNum;
    int arrivalTime;
    //int memory;
};
///==============================
//don't mess with this variable//
int * shmaddr;                 //
//===============================



int getClk()
{
    return *shmaddr;
}


/*
 * All process call this function at the beginning to establish communication between them and the clock module.
 * Again, remember that the clock is only emulation!
*/
void initClk()
{
    int shmid = shmget(SHKEY, 4, 0444);
    while ((int)shmid == -1)
    {
        //Make sure that the clock exists
        printf("Wait! The clock not initialized yet!\n");
        sleep(1);
        shmid = shmget(SHKEY, 4, 0444);
    }
    shmaddr = (int *) shmat(shmid, (void *)0, 0);
}


/*
 * All process call this function at the end to release the communication
 * resources between them and the clock module.
 * Again, Remember that the clock is only emulation!
 * Input: terminateAll: a flag to indicate whether that this is the end of simulation.
 *                      It terminates the whole system and releases resources.
*/

void destroyClk(bool terminateAll)
{
    shmdt(shmaddr);
    if (terminateAll)
    {
        killpg(getpgrp(), SIGINT);
    }
}

struct Queue
{
    int front, rear, size;
    unsigned capacity;
    int *array;
};

// function to create a queue of given capacity.
// It initializes size of queue as 0
struct Queue *createQueue(unsigned capacity)
{
    struct Queue *queue = (struct Queue *)malloc(sizeof(struct Queue));
    queue->capacity = capacity;
    queue->front = queue->size = 0;
    queue->rear = capacity - 1; // This is important, see the enqueue
    queue->array = (int *)malloc(queue->capacity * sizeof(int));
    return queue;
}

// Queue is full when size becomes equal to the capacity
int isFull(struct Queue *queue)
{
    return (queue->size == queue->capacity);
}

// Queue is empty when size is 0
int isEmpty(struct Queue *queue)
{
    return (queue->size == 0);
}

// Function to add an item to the queue.
// It changes rear and size
void enqueue(struct Queue *queue, int item)
{
    if (isFull(queue))
        return;
    queue->rear = (queue->rear + 1) % queue->capacity;
    queue->array[queue->rear] = item;
    queue->size = queue->size + 1;
}

// Function to remove an item from queue.
// It changes front and size
int dequeue(struct Queue *queue)
{
    if (isEmpty(queue))
        return -1;
    int item = queue->array[queue->front];
    queue->front = (queue->front + 1) % queue->capacity;
    queue->size = queue->size - 1;
    return item;
}

// Function to get front of queue
int front(struct Queue *queue)
{
    if (isEmpty(queue))
        return -1;
    return queue->array[queue->front];
}

// Function to get rear of queue
int rear(struct Queue *queue)
{
    if (isEmpty(queue))
        return -1;
    return queue->array[queue->rear];
}

struct nodePCB {
  short processid;
  short arrival;
  int pid;
  short runtime;
  short remainingtime;
  short state;
  short wait;
  short lastexectime;
  short priority;
  short memsize;
  short startingmem;
  short endmem;
  struct nodePCB *next;
};
struct nodePCB *headPCB = NULL;

void insertPCB(struct nodePCB*PCB) {

  if (headPCB == NULL) {
    headPCB = PCB;
    headPCB->next = NULL;
    return;
  }
  PCB->next = headPCB;
  headPCB=PCB;
}
void deletePCB(int id)
{
	if(headPCB==NULL)
	{
		return;
	}
	if(headPCB->processid==id)
	{
		if(headPCB->next==NULL)
		{
			headPCB=NULL;
			return;
		}
		else
		{
			headPCB=headPCB->next;
			return;
		}

	}
	struct nodePCB *temp1 =headPCB;
	struct nodePCB *temp2 =headPCB->next;
	while(temp2!=NULL)
	{
		if(temp2->processid==id)
		{
			temp1->next=temp2->next;
			free(temp2);
			return;
		}
		temp2=temp2->next;
		temp1=temp1->next;
	}
}
struct memory
{
	int starting;
	int end;
	struct memory *next;
};

void insertmem(struct memory**head,int index,int start,int end);
void deleteHead(struct memory**head,int index);
void deleteMemory(struct memory**memory,int index,int starting,int end);

struct memory** memArray;



void insertmem(struct memory**memory,int index,int start,int end)
{
	struct memory * data=(struct memory*)malloc(sizeof(struct memory));
	data->starting=start;
	data->end=end;
	if(memory[index]==NULL)
	{
		memory[index]=data;
		memory[index]->next=NULL;
	}
	else if(memory[index]->next==NULL)
	{
		if(start<memory[index]->starting)
		{
			data->next=memory[index];
			memory[index]=data;
			return;
		}
		else
		{
			memory[index]->next=data;
			data->next=NULL;
		}

	}
	else
	{
		struct memory* ptr1=memory[index];
		struct memory* ptr2=memory[index]->next;
		while(ptr2!=NULL)
		{
			if(start>ptr1->starting && start<ptr2->starting)
			{
				ptr1->next=data;
				data->next=ptr2;
				return;
			}
			ptr1=ptr1->next;
			ptr2=ptr2->next;
		}
		ptr1->next=data;
		data->next=NULL;
	}
}
void deleteHead(struct memory**memory,int index)
{
	if(memory[index]==NULL)
	{
		return;
	}
	memory[index]=memory[index]->next; //we always delete the head;
}
void deleteMemory(struct memory**memory,int index,int starting,int end)
{
	if(memory[index]==NULL)
	{
		return;
	}
	if(memory[index]->starting==starting && memory[index]->end==end)
	{
		if(memory[index]->next==NULL)
		{
			memory[index]=NULL;
			return;
		}
		else
		{
			memory[index]=memory[index]->next;
			return;
		}

	}
	struct memory *temp1 =memory[index];
	struct memory *temp2 =memory[index]->next;
	while(temp2!=NULL)
	{
		if(temp2->starting==starting && temp2->end==end)
		{
			temp1->next=temp2->next;
			free(temp2);
			return;
		}
		temp2=temp2->next;
		temp1=temp1->next;
	}
}


