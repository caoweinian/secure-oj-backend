#ifndef OJ_TIMING_H
#define OJ_TIMING_H

#include <stdint.h>

typedef uint64_t TimeDiffType;
typedef uint64_t MicroSecType;

TimeDiffType timing_start(void);
MicroSecType timing_stop(TimeDiffType start_point);

#endif