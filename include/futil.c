#include "./futil.h"

#include <ctype.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

void fbuf_take_hold(FileBuf *fbuf, int fd) {
    fbuf->fd = fd;
    fbuf_read_block_inner(fbuf);
}

int fbuf_read_ch(FileBuf *fbuf) {
    int ch = EOF;
    if (fbuf->cur != fbuf->end) {
        ch = fbuf->buf[fbuf->cur++];
    } else {
        int16_t read_bytes = fbuf_read_block_inner(fbuf);
        if (read_bytes > 0) {
            ch = fbuf->buf[fbuf->cur++];
        }
    }
    return ch;
}

int copy_file(int fd_from, int fd_to) {
    static char buf[RDWR_BLOCK_SIZE];
    while (true) {
        int bytes = read(fd_from, buf, RDWR_BLOCK_SIZE);
        if (bytes == -1) {
            return -1;
        }
        int write_bytes = write(fd_to, buf, bytes);
        if (write_bytes == -1) {
            return -1;
        }
        if (bytes < RDWR_BLOCK_SIZE) {
            return 0;
        }
    }
}

int oj_file_diff(int fd1, int fd2) {  // no buffer optimization
    static FileBuf buf1;
    static FileBuf buf2;
    fbuf_take_hold(&buf1, fd1);
    fbuf_take_hold(&buf2, fd2);
    int c1, c2;
    bool c1_space = false, c2_space = false;
    while (true) {
        c1 = fbuf_read_ch(&buf1);
        c2 = fbuf_read_ch(&buf2);
        if (c1 == EOF && c2 == EOF) {
            return TWO_FILE_IDENTICAL;
        }
        c1_space = isspace(c1);
        c2_space = isspace(c2);
        if (c1 == EOF) {
            return c2_space ? TWO_FILE_PRESENTATION_ERROR
                            : TWO_FILE_WRONG_ANSWER;
        } else if (c2 == EOF) {
            return c1_space ? TWO_FILE_PRESENTATION_ERROR
                            : TWO_FILE_WRONG_ANSWER;
        } else {
            if ((c1_space ^ c2_space) || (c1_space && c1 != c2)) {
                return TWO_FILE_PRESENTATION_ERROR;
            }
            if (c1 != c2) {
                return TWO_FILE_WRONG_ANSWER;
            }
        }
    }
}

extern void answer_cmp(const JudgeArguments *args, SingleCaseResult *hd);

extern void redirect_stdio(char *in_file_name, char *out_file_name,
                           uint32_t case_no);