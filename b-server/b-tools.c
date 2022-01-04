#include "b-tools.h"
#include <stdio.h>
#include <strings.h> /* bzero */
#include <signal.h>
#include <stdlib.h>  /* exit */
#include <pthread.h> /* pthread_join */
#include <time.h>
#include <sys/time.h>

/* convert bytes to human readable format */
char *bytes_to_human(double bytes, char *buffer)
{
  char *units[] = {"B", "KB", "MB", "GB", "TB", "PB", "EB", "ZB", "YB"};
  int i = 0;
  bzero(buffer, sizeof(*buffer));

  while (bytes >= 1024)
  {
    bytes /= 1024;
    i++;
  }

  sprintf(buffer, "%.*f %s", i ? 2 : 0, bytes, units[i]);
  return buffer;
}

/* simple error check */
int pass(int result, char *msg_error)
{
  if (result == -1)
  {
    fprintf(stderr, "%s\n", msg_error);
    exit(1);
  }
  return result;
}

volatile sig_atomic_t running = 1;

/* signal handler */
void sig_handler(int signo)
{
  printf("Received SIGNAL %d\n", signo);
  running = 0;
}

/* setup signals */
void setup_signals()
{
  struct sigaction signal_action;
  signal_action.sa_handler = sig_handler;
  sigemptyset(&signal_action.sa_mask);
  signal_action.sa_flags = 0;
  sigaction(SIGINT, &signal_action, NULL);
}

/* thread ending */
int wait_threads_end(pthread_t *threads, int n_threads)
{
  int errors = 0;
  int *error = NULL;
  int i;

  for (i = 0; i < n_threads; i++)
  {
    pthread_join(threads[i], (void *)&error);
    errors += *error;
    free(error);
  }

  return errors;
}

/* time */

struct timespec time_diff(struct timespec start, struct timespec end)
{
  struct timespec elapsed;
  elapsed.tv_nsec = end.tv_nsec - start.tv_nsec;
  elapsed.tv_sec = end.tv_sec - start.tv_sec;
  if (elapsed.tv_sec > 0 && elapsed.tv_nsec < 0)
  {
    elapsed.tv_nsec += NANO_TO_SEC;
    elapsed.tv_sec--;
  }
  else if (elapsed.tv_sec < 0 && elapsed.tv_nsec > 0)
  {
    elapsed.tv_nsec -= NANO_TO_SEC;
    elapsed.tv_sec++;
  }
  return elapsed;
}

double time_diff_sec(struct timespec start, struct timespec end)
{
  struct timespec elapsed = time_diff(start, end);
  return (double)elapsed.tv_sec + (elapsed.tv_nsec / NANO_TO_SEC);
}

double time_diff_ms(struct timespec start, struct timespec end)
{
  struct timespec elapsed = time_diff(start, end);
  return (double)(elapsed.tv_sec / 1000) + (elapsed.tv_nsec / NANO_TO_MS);
}

double time_diff_ns(struct timespec start, struct timespec end)
{
  struct timespec elapsed = time_diff(start, end);
  return (double)(elapsed.tv_sec / NANO_TO_SEC) + elapsed.tv_nsec;
}