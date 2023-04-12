#include "./timing.h"

#include <stdlib.h>

#include "./error.h"

#if defined(_WIN32) || defined(_WIN64)
#include <time.h>
#else
#include <sys/time.h>
#endif

#define SOME_PREVIOUS_POINT 1618580000

#if defined(_WIN32) || defined(_WIN64)

static inline TimeDiffType corrected_time(struct timespec *ts) {
    return (ts->tv_sec - SOME_PREVIOUS_POINT) * 1000000 + ts->tv_nsec / 1000;
}

TimeDiffType timing_start(void) {
    struct timespec ts;
    if (timespec_get(&ts, TIME_UTC) == 0) {
        fatal("function \"timespec_get\"\n");
    }
    return corrected_time(&ts);
}

MicroSecType timing_stop(TimeDiffType start_point) {
    struct timespec ts;
    if (timespec_get(&ts, TIME_UTC) == 0) {
        fatal("function \"timespec_get\"\n");
    }
    return corrected_time(&ts) - start_point;
}
#else

static inline TimeDiffType corrected_time(struct timeval *tv) {
    return (tv->tv_sec - SOME_PREVIOUS_POINT) * 1000000 + tv->tv_usec;
}

TimeDiffType timing_start(void) {
    struct timeval tv;
    if (gettimeofday(&tv, NULL) == -1) {
        fatal("gettimeofday\n");
    }
    return corrected_time(&tv);
}

MicroSecType timing_stop(TimeDiffType start_point) {
    struct timeval tv;
    if (gettimeofday(&tv, NULL) == -1) {
        fatal("gettimeofday\n");
    }
    return corrected_time(&tv) - start_point;
}

#endif