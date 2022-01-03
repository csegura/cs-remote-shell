#include "b-tools.h"
#include <stdio.h>
#include <strings.h> /* bzero */
#include <signal.h>
#include <stdlib.h>  /* exit */
#include <pthread.h> /* pthread_join */

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