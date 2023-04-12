#ifndef FUTIL_H
#define FUTIL_H

#include <fcntl.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "./judge.h"
#include "./nums.h"

typedef struct {
    char buf[RDWR_BLOCK_SIZE];
    int fd;
    int16_t cur;
    int16_t end;
} FileBuf;

static inline int16_t fbuf_read_block_inner(FileBuf *fbuf) {
    fbuf->cur = 0;
    int16_t end = read(fbuf->fd, fbuf->buf, RDWR_BLOCK_SIZE);
    if (end == -1) {
        exit(-1);
    }
    return fbuf->end = end;
}

void fbuf_take_hold(FileBuf *fbuf, int fd);

int fbuf_read_ch(FileBuf *buf);

int copy_file(int fd_from, int fd_to);

int oj_file_diff(int fd1, int fd2);

inline void answer_cmp(const JudgeArguments *args, SingleCaseResult *hd) {
    int user_answer_path_fd = open(args->io.indirect_workdir_path, O_DIRECTORY);
    int canonical_answer_path_fd =
        open(args->io.indirect_cases_path, O_DIRECTORY);
    char answer_filename[MAX_UINT32_DEC_BYTES + 5];
    int user_answer_fd, canonical_answer_fd;
    for (uint32_t current_file_no = 0;
         current_file_no != args->limits.case_count; ++current_file_no) {
        if (hd[current_file_no] == WaitForJudging) {
            sprintf(answer_filename, "out_%u", current_file_no + 1);
            user_answer_fd =
                openat(user_answer_path_fd, answer_filename, O_RDONLY);
            canonical_answer_fd =
                openat(canonical_answer_path_fd, answer_filename, O_RDONLY);
            switch (oj_file_diff(user_answer_fd, canonical_answer_fd)) {
                case TWO_FILE_IDENTICAL:
                    hd[current_file_no] = SingleAccepted;
                    break;
                case TWO_FILE_WRONG_ANSWER:
                    hd[current_file_no] = SingleWrongAnswer;
                    break;
                case TWO_FILE_PRESENTATION_ERROR:
                    hd[current_file_no] = SinglePresentationError;
                    break;
            }
            close(user_answer_fd);
            close(canonical_answer_fd);
        }
    }
    close(user_answer_path_fd);
    close(canonical_answer_path_fd);
}

inline void redirect_stdio(char *in_file_name, char *out_file_name,
                           uint32_t case_no) {
    sprintf(in_file_name, "in_%u", case_no);
    sprintf(out_file_name, "out_%u", case_no);
    int out_file_fd = open(out_file_name, O_CREAT | O_WRONLY, S_IRUSR | S_IWUSR);
    fchown(out_file_fd, LIMITED_USERID, LIMITED_USERID);
    int in_file_fd = open(in_file_name, O_RDONLY);
    // int sanitize_fd = open("address", O_CREAT | O_WRONLY, S_IRUSR | S_IWUSR);
    // fchown(sanitize_fd, LIMITED_USERID, LIMITED_USERID);
    dup2(in_file_fd, STDIN_FILENO);
    dup2(out_file_fd, STDOUT_FILENO);
    // dup2(sanitize_fd, STDERR_FILENO);
    close(in_file_fd);
    close(out_file_fd);
    // close(sanitize_fd);
}

#endif