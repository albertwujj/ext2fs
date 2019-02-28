//
//  c4.1.c
//  
//
//  Created by Albert Wu on 2/9/19.
//

#include "c4.1.h"

/**** C4.1.c file: compute matrix sum by threads ***/
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <pthread.h>

#define  M   4
#define  N   50000

int A[M][N];

int total;
pthread_mutex_t m;

void *func(void *arg)        // threads function
{
  int temp, mysum, j = 0;

  for (j=0; j < N; j++)     // compute sum of A[row]in global sum[row]
    mysum += A[(int) arg][j];

  pthread_mutex_lock(&m);
  /************** A CRITICAL REGION *******************/
  temp = total;   // get total
  temp += mysum;  // add mysum to temp
  sleep(1);       // OR for (int i=0; i<100000000; i++); ==> switch threads
  total = temp;   //  write temp to total
  /************ end of CRITICAL REGION ***************/
  pthread_mutex_unlock(&m);

  pthread_exit((void*)0);  // thread exit: 0=normal termination
}

// print the matrix (if N is small, do NOT print for large N)
int print()
{
  int i, j;
  for (i=0; i < M; i++){
    for (j=0; j < N; j++){
      printf("%4d ", A[i][j]);
    }
    printf("\n");
  }
}

int main (int argc, char *argv[])
{
  struct timeval begin, end;
  gettimeofday(&begin, NULL);
  pthread_t thread[M];      // thread IDs
  int i, j, status;

  printf("main: initialize A matrix\n");

  for (i=0; i < M; i++){
    for (j=0; j < N; j++){
      A[i][j] = i + j + 1;
    }
  }

  //print();

  pthread_mutex_init(&m, NULL);

  printf("main: create %d threads\n", M);
  for(i=0; i < M; i++) {
    pthread_create(&thread[i], NULL, func, (void *)i);
  }

  printf("main: try to join with threads\n");
  for(i=0; i < M; i++) {
    pthread_join(thread[i], (void *)&status);
    printf("main: joined with thread %d : status=%d\n", i, status);
  }

  

  printf("tatal = %ld\n", total);

  gettimeofday(&end, NULL);
  double total_time =  (end.tv_usec - begin.tv_usec) / 1000000.0;
  printf("total running time: %lf\n", total_time);
  pthread_exit(NULL);

}
