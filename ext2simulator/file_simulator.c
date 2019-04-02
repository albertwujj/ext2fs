//
//  file_simulator.c
//  
//
//  Created by Albert Wu on 3/7/19.
//

#include <stdio.h>
int fs_init()
{
  int i,j;
  for (i=0; i<NMINODE; i++) // initialize all minodes as FREE
    minode[i].refCount = 0;
  for (i=0; i<NMTABLE; i++) // initialize mtables as FREE
    mtable[i].dev = 0;
  for (i=0; i<NOFT; i++) // initialize ofts as FREE
    oft[i].refCount = 0;
  for (i=0; i<NPROC; i++){ // initialize PROCs
    proc[i].status = READY; // ready to run
    proc[i].pid = i; // pid = 0 to NPROC-1
    proc[i].uid = i; // P0 is a superuser process
  for (j=0; j<NFD; j++)
    proc[i].fd[j] = 0; // all file descriptors are NULL
    proc[i].next = &proc[i+1]; // link list
  }
  proc[NPROC-1].next = &proc[0]; // circular list
  running = &proc[0]; // P0 runs first
}
