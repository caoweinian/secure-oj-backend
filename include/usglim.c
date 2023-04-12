#include "./usglim.h"

uint32_t get_alarm_secs_upper_bound(uint32_t limit_ms) {
    if (limit_ms % 1000 != 0) {
        limit_ms += 1000;
    }
    int limit_sec = limit_ms /= 1000;
    return 2 + limit_sec;
}

uint32_t get_space_limit_bytes(uint32_t limit_kb) {
    uint32_t limit_bytes = limit_kb * 1024;
    uint32_t rem = limit_bytes % 4096;
    if (rem != 0) {
        limit_bytes = limit_bytes - rem + 4096;
    }
    return limit_bytes;
}

uint32_t get_total_time_milli(struct rusage *ru) {
    uint32_t t1 = ru->ru_utime.tv_sec * 1000000 + ru->ru_utime.tv_usec;
    uint32_t t2 = ru->ru_stime.tv_sec * 1000000 + ru->ru_stime.tv_usec;
    return (t1 + t2) / 1000;
}

extern void set_limits(const JudgeArguments *args, uint64_t twice_file_size);