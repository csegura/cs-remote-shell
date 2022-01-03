#ifndef B_TOOLS_H
#define B_TOOLS_H
#include <signal.h>

/* macros for elapsed time */
#define ELAPSED_MS ((double)(end - start) / (CLOCKS_PER_SEC / 1000))
#define ELAPSED_S (double)(end - start)

#define SPEED_MS(size) ((size / ((double)(end - start) / CLOCKS_PER_SEC)) / (1024 * 1024))
#define SPEED_S(size) (((size) / (double)(end - start)) / (1024 * 1024))

/* utils */
char *bytes_to_human(double bytes, char *buffer);
int pass(int result, char *msg_error);

/* threads */
extern volatile sig_atomic_t running;
int wait_threads_end(pthread_t *threads, int n_threads);

/* signals */
void setup_signals();

#endif
