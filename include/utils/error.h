#ifndef OJ_ERROR_H
#define OJ_ERROR_H

/// 堆分配strerror结果。若返回NULL，则代表内存耗尽
char *heap_allocated_strerror(void);

void fatal(const char *fmt, ...);

inline void fatal_alloc(void) { fatal("out of memory\n"); }

#endif