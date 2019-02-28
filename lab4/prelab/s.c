//
//  s.c
//  
//
//  Created by Albert Wu on 2/9/19.
//

#include <stdio.h>
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
  int total = 0;
  printf("main: initialize A matrix\n");
  int i, j = 0;
  for (i=0; i < M; i++){
    for (j=0; j < N; j++){
      A[i][j] = i + j + 1;
    }
  }
  for (i=0; i < M; i++){
    for (j=0; j < N; j++){
      total += A[i][j];
    }
  }
  printf("total: %d\n", total);

  
  gettimeofday(&end, NULL);
  double total_time =  (float) (end.tv_sec - begin.tv_sec) + (end.tv_usec - begin.tv_usec) / 1000000.0;
  printf("total running time: %lf\n", total_time);
  pthread_exit(NULL);

}
