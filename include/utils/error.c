#include "./error.h"

#include <errno.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

char *heap_allocated_strerror(void) {
    char *static_allocated = strerror(errno);
    uint64_t len = strlen(static_allocated);
    char *heap_allocated = (char *)malloc(len + 1);
    if (heap_allocated == NULL) {
        return NULL;
    }
    strcpy(heap_allocated, static_allocated);
    return heap_allocated;
}

void fatal(const char *fmt, ...) {
    va_list vp;
    fprintf(stderr, "fatal: ");
    va_start(vp, fmt);
    vfprintf(stderr, fmt, vp);
    va_end(vp);
    exit(EXIT_FAILURE);
}

extern void fatal_alloc(void);