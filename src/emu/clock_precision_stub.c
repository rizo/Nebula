/**
 *  @author Jesse Haber-Kucharsky
 *  @see 'LICENSE' License details
 */

#include <caml/alloc.h>
#include <caml/memory.h>
#include <caml/mlvalues.h>

#include <time.h>

#ifdef __MACH__
#include <mach/mach_time.h>
#endif

#define NANOSECONDS_PER_SECOND 1000000000

CAMLprim value nebula_clock_precision_get_time(value unit) {
  CAMLparam1(unit);

  long nanoseconds;

#ifdef __MACH__
  nanoseconds = mach_absolute_time();
#else
  struct timespec ts;

  // Assume success.
  clock_gettime(CLOCK_REALTIME, &ts);

  nanoseconds = (NANOSECONDS_PER_SECOND * ts.tv_sec) + ts.tv_nsec;
#endif

  CAMLreturn(caml_copy_int64(nanoseconds));
}
