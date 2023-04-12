#ifndef OJ_MONITOR_H
#define OJ_MONITOR_H

#include <stdint.h>
#include <unistd.h>

#include "./judge.h"
#include "./whitelist.h"

typedef struct {
    union {
        int32_t invalid_syscall;
        struct {
            uint64_t peak_memory_bytes;
            uint64_t cpu_use_milli;
        } usg;
    } inner;
    SingleCaseResult single_result;
} MonitorUsage;

MonitorUsage monitor(const JudgeArguments *args, Whitelist *wl, pid_t child);

#endif