#include <caml/alloc.h>
#include <caml/mlvalues.h>

#include <time.h>

#define NANOSECONDS_PER_SECOND 1000000000

CAMLprim value nebula_precision_clock_get_time(void) {
  struct timespec ts;

  // Assume success.
  clock_gettime(CLOCK_REALTIME, &ts);

  long nanoseconds = (NANOSECONDS_PER_SECOND * ts.tv_sec) + ts.tv_nsec;
  return Val_long(nanoseconds);
}
