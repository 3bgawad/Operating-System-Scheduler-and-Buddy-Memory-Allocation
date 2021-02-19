# Operating-System-Scheduler-and-Buddy-Memory-Allocation
Implementation of Operating System scheduling algorithms including Round Robin, STRN, and HPF. and Implementation of Buddy Memory Allocation

processgenerator.c 
* Reads processes from a text file (process.txt) and add them to a queue and when their time comes to be added with the scheduled processes , it is sent to scheduler.c process via message queue

process.txt
* We read the process info from this file which are the Process ID, arrival time, runtime, priority and memory size needed 

clk.c
* it is the process simulating the clock of the system

process.c
* the schedule is supposed to run each process.c file in a different thread and manage them depending on the scheduling algorithm

scheduler.c
* 3 scheduling algorithms available (STRN, Round Robin, Highest Priority First)
* Buddy Memory Allocation for a 1024KB memory, the amount of memory each process need is read from process.txt
