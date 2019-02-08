int tswitch();

int sleep(int event)
{
  printf("proc %d going to sleep on event=%d\n", running->pid, event);

  running->event = event;
  running->status = SLEEP;
  enqueue(&sleepList, running);
  printList("sleepList", sleepList);
  tswitch();
}

int wakeup(int event)
{
  PROC *temp, *p;
  temp = 0;
  printList("sleepList", sleepList);

  while (p = dequeue(&sleepList)){
     if (p->event == event){
	printf("wakeup %d\n", p->pid);
	p->status = READY;
	enqueue(&readyQueue, p);
     }
     else{
	enqueue(&temp, p);
     }
  }
  sleepList = temp;
  printList("sleepList", sleepList);
}


int wait(int *status) {
  if (!running->child) {
    return -1;
  }
  while (1) {
    PROC *c = running->child;
    PROC *prev = running;
    while(c) {
      if(c->status == ZOMBIE) {
        *status = c->exitCode;
        if(prev == running) {
          running->child = c->sibling;
        } else {
          prev->sibling = c->sibling;
        }
        c->sibling = NULL;
        c->status = FREE;
        enqueue(&freeList, c);
        return c->pid;
      }
      prev = c;
      c = c->sibling;
    }
    sleep(running);
  }
}

int kexit(int exitValue)
{
  printf("proc %d in kexit(), value=%d\n", running->pid, exitValue);
  PROC *p1 = proc + 1;
  PROC *rc = running->child;
  PROC *pc = p1->child;
  if (!pc) {
    p1->child = rc;
  } else {
    while (pc->sibling) {
      pc = pc->sibling;
    }
    pc->sibling = rc;
  }
  while (rc) {
    printf("wh\n");
    rc->parent = p1;
    rc = rc->sibling;
  }

  running->exitCode = exitValue;
  running->status = ZOMBIE;
  wakeup(running->parent);
  tswitch();
}
