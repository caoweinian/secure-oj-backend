#include "./monitor.h"

#include <stdbool.h>
#include <string.h>
#include <sys/ptrace.h>
#include <sys/reg.h>
#include <sys/resource.h>
#include <sys/wait.h>
#include <unistd.h>


#include <stdio.h>

#include "./nums.h"

MonitorUsage monitor(const JudgeArguments *args, Whitelist *wl, pid_t child) {
    MonitorUsage usg = {
        .single_result = WaitForJudging,
    };
    uint64_t peak_bytes = 0;
    uint64_t current_bytes = 0;
    int32_t syscall = 0;
    struct rusage ru;
    int status = 0;
    int stopped_status = 0;
    bool is_caught_by_invalid_ops = false;
    // bool first = true;

    while (true) {
        wait4(child, &status, 0, &ru);
        // if(first){
        //      ptrace(PTRACE_SETOPTIONS, child, NULL, PTRACE_O_TRACESYSGOOD |
        //      PTRACE_O_TRACEEXIT);
        // }
        current_bytes = ru.ru_minflt * getpagesize();
        if (current_bytes > peak_bytes) {
            peak_bytes = current_bytes;
        }
        if (peak_bytes > args->limits.max_space_limit_mb * MEGA_BYTES) {
            usg.single_result = SingleSpaceLimitExceeded;
            kill(child, SIGKILL);
            wait4(child, &status, 0, &ru);
            break;
        }
        if (WIFEXITED(status)) {
            break;
        }
        if (WIFSTOPPED(status)) {
            stopped_status = WSTOPSIG(status);
            if (stopped_status == SIGCHLD || stopped_status == SIGTRAP ||
                stopped_status == (0x80 | SIGTRAP) || stopped_status == 0)
                ;  // just go on.
            else {
                if (stopped_status == SIGALRM) {
                    alarm(0);
                } else if (stopped_status == SIGKILL ||
                           stopped_status == SIGXCPU) {
                    usg.single_result = SingleTimeLimitExceeded;
                } else if (stopped_status == SIGXFSZ) {
                    usg.single_result = SingleWrongAnswer;
                } else if (stopped_status == SIGSEGV) {
                    usg.single_result = SingleSpaceLimitExceeded;
                } else {
                    fprintf(stderr, "debug: encountered %s\n", strsignal(WTERMSIG(status)));
                    usg.single_result = SingleRuntimeError;  // ?
                    break;
                }
                kill(child, SIGKILL);
                wait4(child, &status, 0, &ru);
                break;
            }
        }
        if (WIFSIGNALED(status)) {
            switch (WTERMSIG(status)) {
                case SIGCHLD:
                case SIGALRM:
                    alarm(0);
                case SIGXCPU:
                    usg.single_result = SingleTimeLimitExceeded;
                    break;
                case SIGKILL:  // RLIMIT_CPU/syscall fails
                    usg.single_result =
                        (is_caught_by_invalid_ops ? SingleOperationNotPermitted
                                                  : SingleTimeLimitExceeded);
                    break;
                case SIGXFSZ:
                    usg.single_result = SingleWrongAnswer;
                    break;
                default:
                    fprintf(stderr, "debug: encountered %s\n", strsignal(WTERMSIG(status)));
                    usg.single_result = SingleRuntimeError;
            }
            break;
        }
        syscall =
            ptrace(PTRACE_PEEKUSER, child, sizeof(void *) * ORIG_RAX, NULL);
        if (whitelist_contains(wl, syscall)) {
            if (syscall == SYS_execve) {
                whitelist_erase(wl, SYS_execve);
            }
        } else {
            usg.single_result = SingleOperationNotPermitted;
            usg.inner.invalid_syscall = syscall;
            is_caught_by_invalid_ops = true;
            kill(child, SIGKILL);
            // return usg;
        }
        ptrace(PTRACE_SYSCALL, child, NULL, NULL);
        // first = false;
    }
    usg.inner.usg.cpu_use_milli +=
        (ru.ru_utime.tv_sec * 1000 + ru.ru_utime.tv_usec / 1000);
    usg.inner.usg.cpu_use_milli +=
        (ru.ru_stime.tv_sec * 1000 + ru.ru_stime.tv_usec / 1000);
    usg.inner.usg.peak_memory_bytes = peak_bytes;
    return usg;
}