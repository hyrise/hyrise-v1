
#include "epoch.h"

#ifdef __APPLE__
#include <mach/mach_time.h>
#else
#include <time.h>
#include <sys/time.h>
#include <features.h>
#endif

epoch_t get_epoch_nanoseconds() {
  epoch_t epoch = 0;

#ifdef __APPLE__

  mach_timebase_info_data_t info;
  mach_timebase_info(&info);

  uint64_t start = mach_absolute_time();

  start *= info.numer;
  start /= info.denom;

  epoch = start;

#else


#if _POSIX_C_SOURCE >= 199309L
  struct timespec time;
  clock_gettime(CLOCK_MONOTONIC, &time);
  epoch = time.tv_nsec + time.tv_sec * 1000000000;
#else
  struct timeval time;
  gettimeofday(&time, 0);
  epoch = (time.tv_usec + time.tv_sec * 1000000) * 1000;
#endif


#endif

  return epoch;
}
