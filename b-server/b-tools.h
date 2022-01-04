#ifndef B_TOOLS_H
#define B_TOOLS_H
#include <signal.h>

/* macros for elapsed time */

#define NANO_TO_SEC 1000000000.0
#define NANO_TO_MS 1000000.0

#define ElapsedStart()   \
  struct timespec start; \
  clock_gettime(CLOCK_REALTIME, &start);

#define ElapsedEnd()   \
  struct timespec end; \
  clock_gettime(CLOCK_REALTIME, &end);

#define ELAPSED_MS time_diff_ms(start, end)
#define ELAPSED_SEC time_diff_sec(start, end)
#define ELAPSED_NS time_diff_ns(start, end)

#define MB_SIZE (1024 * 1024)
#define SPEED_MBS(size) (((size) / ELAPSED_SEC) / MB_SIZE)

/* utils */
char *bytes_to_human(double bytes, char *buffer);
int pass(int result, char *msg_error);

/* threads */
extern volatile sig_atomic_t running;
int wait_threads_end(pthread_t *threads, int n_threads);

/* signals */
void setup_signals();

/* time */
long time_nanos();

double time_diff_sec(struct timespec start, struct timespec end);
double time_diff_ms(struct timespec start, struct timespec end);
double time_diff_ns(struct timespec start, struct timespec end);
#endif
