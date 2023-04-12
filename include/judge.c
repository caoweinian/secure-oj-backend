#include "./judge.h"

#include <dirent.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ptrace.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <unistd.h>

#include "./futil.h"
#include "./monitor.h"
#include "./usglim.h"

static inline void setguid(void) {
    setgid(LIMITED_USERID);
    setuid(LIMITED_USERID);
}

static inline AllCasesResult final_judge(const SingleCaseResult *results,
                                         uint32_t case_count) {
    uint32_t right_cnt = 0;
    uint32_t wrong_cnt = 0;
    for (uint32_t i = 0; i != case_count; ++i) {
        if (results[i] == SingleAccepted) {
            ++right_cnt;
        } else {
            ++wrong_cnt;
        }
        if (right_cnt != 0 && wrong_cnt != 0) {
            return AllPartialAccepted;
        }
    }
    return right_cnt != 0 ? AllAccepted : AllNotAccepted;
}

static inline JudgeResult *new_result() {
    JudgeResult *ret = (JudgeResult *)malloc(sizeof(JudgeResult));
    if (ret == NULL) {
        return NULL;
    }
    ret->code = InternalStateOk;
    ret->prompt_message = NULL;
    ret->final = Undefined;
    ret->single_length = 0;
    ret->single_results = NULL;
    ret->any = NULL;
    return ret;
}

JudgeResult *judge(const JudgeArguments *args) {
    JudgeResult *result = new_result();
    if (result == NULL) {
        return NULL;
    }

    if (getuid() != 0) {
        result->code = ImNotRoot;
        return result;
    }
    // remember to close it
    int source_code_relative_path_fd =
        open(args->io.indirect_workdir_path, O_DIRECTORY, 0700);
    // if (source_code_relative_path_fd == -1) {
    //     result->code = InvalidArgument;
    //     return result;
    // }
    {
        uint64_t file_size = 0;
        struct stat meta;
        fstatat(source_code_relative_path_fd, args->io.filename, &meta, 0);
        file_size = meta.st_size;
        if (file_size > args->limits.max_code_bytes) {
            close(source_code_relative_path_fd);
            result->final = SourceTooLong;
            char *exact_file_size_as_str = (char *)malloc(
                1 + MAX_UINT64_DEC_BYTES); 
            if (exact_file_size_as_str == NULL) {
                result->code = OutOfMemory;
                return result;
            }
            sprintf(exact_file_size_as_str, "%lu", file_size);
            result->prompt_message = exact_file_size_as_str;
            return result;
        }
    }
    // 编译源文件。
    {
        pid_t child = fork();
        if (child < 0) {
            close(source_code_relative_path_fd);
            result->code = InternalError;
            return result;
        } else if (child == 0) {
            alarm(5);  //  limit compilation time strictly within 5 secs
            // set gid and uid to 1000.
            // 1000 is current linux user and hard-encoded;
            // when we do extensions in the future, we can use specialized user number(like 1536) or nobody.
            // now it is just for convenience
            setguid();
            int cpe_err_info_fd = openat(source_code_relative_path_fd,
                                         "cpe_info", O_CREAT | O_WRONLY, S_IRUSR | S_IWUSR);
            dup2(cpe_err_info_fd, STDERR_FILENO);
            close(cpe_err_info_fd);
            chdir(args->io.indirect_workdir_path);
            switch (args->language) {
                case C:
                    execl("/usr/bin/gcc", "/usr/bin/gcc", args->io.filename,
                          "-std=c11", "-O1", "-w", "-fmax-errors=1", "-static",
                          "-o", "obj", /*"-fsnitize=address",*/ NULL);
                    break;
                case Cplusplus:
                    execl("/usr/bin/g++", "/usr/bin/g++", args->io.filename,
                          "-std=c++2a", "-O1", "-w", "-fmax-errors=1",
                          "-static", "-o", "obj", /*"-fsanitize=address",*/ NULL);
                    break;
            }
            exit(-1);
        } else {
            int status = 0;
            waitpid(child, &status, 0);
            if (!WIFEXITED(status)) {
                if (WIFSTOPPED(status)) {
                    int sig = WSTOPSIG(status);
                    if (sig == SIGALRM) {  // compilation time exceeded
                        result->final = CompilationTimeExceeded;
                        return result;
                    }
                    result->code = InternalError;
                    return result;
                }
            }
            int cpe_err_info_fd = openat(source_code_relative_path_fd,
                                         "cpe_info", O_RDONLY, S_IRUSR);
            struct stat err_file_stat;
            fstat(cpe_err_info_fd, &err_file_stat);
            char *stderr_buf = (char *)malloc(sizeof(char) * 1001);
            if (stderr_buf == NULL) {
                result->code = OutOfMemory;
                return result;
            }
            ssize_t read_bytes = read(cpe_err_info_fd, stderr_buf, 1000);
            if (read_bytes == -1) {
                close(cpe_err_info_fd);
                free(stderr_buf);
                result->code = InternalError;
                return result;
            } else if (read_bytes > 0) {
                result->final = CompilationError;
                stderr_buf[read_bytes] = '\0';
                result->prompt_message = stderr_buf;
                close(cpe_err_info_fd);
                return result;
            } else {
                free(stderr_buf);
                close(cpe_err_info_fd);
            }
        }
    }
    close(source_code_relative_path_fd);
    // file_size_2_times
    uint64_t *limit_table =
        (uint64_t *)malloc(sizeof(uint64_t) * args->limits.case_count);
    if (limit_table == NULL) {
        result->code = OutOfMemory;
        return result;
    }
    // 将输入样例copy到jail目录中
    {
        int fds[2];
        pipe(fds);
        pid_t child = fork();
        if (child < 0) {
            result->code = InternalError;
            return result;
        } else if (child == 0) {
            setguid();
            int cases_origin_dir_fd =
                open(args->io.indirect_cases_path, O_DIRECTORY);
            int copy_to_dir_fd =
                open(args->io.indirect_workdir_path, O_DIRECTORY);
            //  in: in__{n} , out: out_{n}
            char input_from_to_name[MAX_UINT32_DEC_BYTES + 4];
            struct stat stat_buf;
            for (uint32_t i = 1; i <= args->limits.case_count; ++i) {
                fflush(stderr);
                sprintf(input_from_to_name, "in_%u", i);
                int from_fd =
                    openat(cases_origin_dir_fd, input_from_to_name, O_RDONLY);
                fstat(from_fd, &stat_buf);
                uint64_t twice_file_size = stat_buf.st_size * 2;
                write(fds[1], (void *)(&twice_file_size), sizeof(uint64_t));
                int to_fd = openat(copy_to_dir_fd, input_from_to_name,
                                   O_WRONLY | O_CREAT, S_IRUSR);
                copy_file(from_fd, to_fd);
                fchmod(to_fd, S_IRUSR);

                // to_fd -> read only
                close(from_fd);
                close(to_fd);
            }
            exit(0);
        } else {
            int status = 0;
            wait(&status);
            if (!WIFEXITED(status)) {
                result->code = InternalError;
                return result;
            }
            uint64_t twice_file_size = 0;
            for (uint32_t i = 0; i != args->limits.case_count; ++i) {
                read(fds[0], (void *)(&twice_file_size), sizeof(uint64_t));
                limit_table[i] = twice_file_size;
            }
            close(fds[0]);
            close(fds[1]);
        }
    }
    // start monitoring
    char origin_absolute_path[PATH_MAX];
    {
        Whitelist w_list;
        whitelist_init(&w_list);
        getcwd(origin_absolute_path, PATH_MAX);
        chdir(args->io.indirect_workdir_path);
        SingleCaseResult *result_array = (SingleCaseResult *)malloc(
            args->limits.case_count * sizeof(SingleCaseResult));
        if (result_array == NULL) {
            result->code = OutOfMemory;
            return result;
        }
        for (uint32_t cur = 0; cur != args->limits.case_count; ++cur) {
            result_array[cur] = WaitForJudging;
        }
        result->single_length = args->limits.case_count;
        result->single_results = result_array;
        char in_file_name[MAX_UINT32_DEC_BYTES + 4];   // in_{n}
        char out_file_name[MAX_UINT32_DEC_BYTES + 5];  // out_{n}
        for (uint32_t current_case_no = 1;
             current_case_no <= args->limits.case_count; ++current_case_no) {
            pid_t child = fork();
            if (child < 0) {
                result->code = InternalError;
                return result;
            } else if (child == 0) {
                redirect_stdio(in_file_name, out_file_name, current_case_no);
                set_limits(args, limit_table[current_case_no - 1]);
                chroot(".");
                setguid();
                ptrace(PTRACE_TRACEME, 0, NULL, NULL);
                execl("/obj", "./obj", NULL);
                exit(-1);
            } else {
                whitelist_reset(&w_list);
                MonitorUsage usg = monitor(args, &w_list, child);
                if (usg.single_result != WaitForJudging) {
                    result->single_results[current_case_no - 1] =
                        usg.single_result;
                }
            }
        }
    }
    // file comparing
    SingleCaseResult *hd = result->single_results;
    chdir(origin_absolute_path);
    answer_cmp(args, hd);
    // file comparing end
    result->final = final_judge(hd, args->limits.case_count);

    free(limit_table);
    return result;
}

void free_judge_result(JudgeResult *result) {
    if (result != NULL) {
        free(result->prompt_message);
        free(result->any);
        free(result->single_results);
        free(result);
    }
}