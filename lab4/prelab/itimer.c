#include <stdio.h>
#include <string.h>
#include <signal.h>
#include <sys/time.h>
#include <stdlib.h>

/*************************
 struct timeval {
 time_t      tv_sec;         // seconds
 suseconds_t tv_usec;        // microseconds
 };
 struct itimerval {
 struct timeval it_interval; // Interval of periodic timer
 struct timeval it_value;    // Time until next expiration
 };
 *********************/

int hh, mm, ss, tick;

void timer_handler (int sig)
{
  if (!tick) {
    tick = 500;
  }
  if((tick % 1000) == 0) {
    ss += 1;
    mm = ss / 60;
    hh = mm / 60;
    system("clear");
    printf("%d : %d : %d\n", hh, mm % 60, ss % 60);
    tick = 0;
  }
  tick++;
}

int main ()
{
  struct itimerval timer;
  tick = hh = mm = ss = 0;

  signal(SIGALRM, &timer_handler);

  /* Configure the timer to expire after 1 sec */
  timer.it_value.tv_sec  = 0;
  timer.it_value.tv_usec = 500000;

  /* and every 1 sec after that */
  timer.it_interval.tv_sec  = 0;
  timer.it_interval.tv_usec = 1000;

  setitimer (ITIMER_REAL, &timer, NULL);

  while (1);
}
