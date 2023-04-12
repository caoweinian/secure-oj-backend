#ifndef OJ_USGLIM_H
#define OJ_USGLIM_H

#include <stdint.h>
#include <stdio.h>
#include <sys/resource.h>
#include <unistd.h>

#include "./judge.h"
#include "./nums.h"

uint32_t get_alarm_secs_upper_bound(uint32_t limit_ms);

uint32_t get_space_limit_bytes(uint32_t limit_kb);

uint32_t get_total_time_milli(struct rusage *ru);

inline void set_limits(const JudgeArguments *args, uint64_t twice_file_size) {
    struct rlimit rl;
    uint32_t alarm_time =
        get_alarm_secs_upper_bound(args->limits.max_time_limit_milli);
    // rl.rlim_max = rl.rlim_cur = 1;  // for debug
    rl.rlim_max = rl.rlim_cur = alarm_time;
    setrlimit(RLIMIT_CPU, &rl);
    rl.rlim_max = rl.rlim_cur = args->limits.max_space_limit_mb * MEGA_BYTES;
    // rl.rlim_max = rl.rlim_cur = 256 * MEGA_BYTES; // for debug
    setrlimit(RLIMIT_AS, &rl);
    rl.rlim_max = rl.rlim_cur = 64 * MEGA_BYTES;
    setrlimit(RLIMIT_STACK, &rl);
    rl.rlim_max = rl.rlim_cur = twice_file_size;
    setrlimit(RLIMIT_FSIZE, &rl);
    // rl.rlim_max = rl.rlim_cur = 3;         // make sense when uid & gid is
    // not "me" int v = setrlimit(RLIMIT_NPROC, &rl);
    // rl.rlim_max = rl.rlim_cur = 0;
    // setrlimit(RLIMIT_NOFILE, &rl);
    alarm(alarm_time + 1);
}

#endif