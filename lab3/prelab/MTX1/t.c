/*********** t.c file of A Multitasking System *********/
#include <stdio.h>
#include "type.h"
#include "string.h"

PROC proc[NPROC];      // NPROC PROCs
PROC *freeList;        // freeList of PROCs 
PROC *readyQueue;      // priority queue of READY procs
PROC *running;         // current running proc pointer

#include "queue.c"     // include queue.c file

/*******************************************************
  kfork() creates a child process; returns child pid.
  When scheduled to run, child PROC resumes to body();
********************************************************/
int body();
int tswitch();

int kfork()
{
  int  i;
  PROC *p = dequeue(&freeList);
  if (!p){
     printf("no more proc\n");
     return(-1);
  }
  /* initialize the new proc and its stack */
  p->status = READY;
  p->priority = 1;       // ALL PROCs priority=1,except P0
  p->ppid = running->pid;
  
  /************ new task initial stack contents ************
   kstack contains: |retPC|eax|ebx|ecx|edx|ebp|esi|edi|eflag|
                      -1   -2  -3  -4  -5  -6  -7  -8   -9
  **********************************************************/
  for (i=1; i<10; i++)               // zero out kstack cells
      p->kstack[SSIZE - i] = 0;
  p->kstack[SSIZE-1] = (int)body;    // retPC -> body()
  p->ksp = &(p->kstack[SSIZE - 9]);  // PROC.ksp -> saved eflag 
  enqueue(&readyQueue, p);           // enter p into readyQueue
  return p->pid;
}

int kexit(int exitCode)
{
   running->status = ZOMBIE;
   tswitch();
}

int do_kfork()
{
   int child = kfork();
   if (child < 0)
      printf("kfork failed\n");
   else{
      printf("proc %d kforked a child = %d\n", running->pid, child); 
      printList("readyQueue", readyQueue);
   }
   return child;
}

int do_switch()
{
   tswitch();
}

int do_exit()
{
  kexit(running->pid); 
}

int do_jesus()
{
  int i;
  PROC *p;
  printf("Jesus performs miracles\n");
  for (i=1; i<NPROC; i++){
    p = &proc[i];
    if (p->status == ZOMBIE){
      p->status = READY;
      enqueue(&readyQueue, p);
      printf("raised a ZOMBIE %d to live again\n", p->pid);
    }
  }
  printList("readyQueue", readyQueue);
}

int menu()
{
  printf("********************************\n");
  printf(" ps  fork  switch  exit  jesus  \n");
  printf("********************************\n");
}

char *status[ ] = {"FREE", "READY", "SLEEP", "ZOMBIE"};
int do_ps()
{
  int i;
  PROC *p;
  printf("PID  PPID  status\n");
  printf("---  ----  ------\n");
  for (i=0; i<NPROC; i++){
    p = &proc[i];
    printf(" %d    %d    ", p->pid, p->ppid);
    if (p == running)
      printf("RUNNING\n");
    else
      printf("%s\n", status[p->status]);
  }
}
    
int body()   // process body function
{
  int c;
  char cmd[64];
  printf("proc %d starts from body()\n", running->pid);
  while(1){
    printf("***************************************\n");
    printf("proc %d running: parent=%d\n", running->pid,running->ppid);
    printList("readyQueue", readyQueue);
    menu();
    printf("enter a command : ");
    fgets(cmd, 64, stdin);
    cmd[strlen(cmd)-1] = 0;

    if (strcmp(cmd, "ps")==0)
      do_ps();
    if (strcmp(cmd, "fork")==0)
      do_kfork();
    if (strcmp(cmd, "switch")==0)
      do_switch();
    if (strcmp(cmd, "exit")==0)
      do_exit();
    if (strcmp(cmd, "jesus")==0)
      do_jesus();
  }
}
// initialize the MT system; create P0 as initial running process
int init() 
{
  int i;
  PROC *p;
  for (i=0; i<NPROC; i++){ // initialize PROCs to freeList
    p = &proc[i];
    p->pid = i;            // PID = 0 to NPROC-1  
    p->status = FREE;
    p->priority = 0;      
    p->next = p+1;
  }
  proc[NPROC-1].next = 0;  
  freeList = &proc[0];     // all PROCs in freeList     
  readyQueue = 0;          // readyQueue = empty

  // create P0 as the initial running process
  p = running = dequeue(&freeList); // use proc[0] 
  p->status = READY;
  p->priority = 0;         // P0 has the lowest priority 0
  p->ppid = 0;             // P0 is its own parent
  printList("freeList", freeList);
  printf("init complete: P0 running\n"); 
}

/*************** main() function ***************/
int main()
{
   printf("Welcome to the MT Multitasking System\n");
   init();    // initialize system; create and run P0
   kfork();   // kfork P1 into readyQueue  
   while(1){
     printf("P0: switch process\n");
     while (readyQueue == 0); // loop if readyQueue empty
     tswitch();
   }
}

/*********** scheduler *************/
int scheduler()
{ 
  printf("proc %d in scheduler()\n", running->pid);
  if (running->status == READY)
     enqueue(&readyQueue, running);
  printList("readyQueue", readyQueue);
  running = dequeue(&readyQueue);
  printf("next running = %d\n", running->pid);  
}
