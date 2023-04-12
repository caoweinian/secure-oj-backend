#ifndef OJ_UNITEST_H
#define OJ_UNITEST_H

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "./error.h"
#include "./nums.h"
#include "./timing.h"

#define GREEN_STR "\033[32m"
#define RED_STR "\033[31m"
#define DEFAULT_STR "\033[0m"

inline void GREEN_MODE(void) { printf("\033[32m"); }

inline void RED_MODE(void) { printf("\033[31m"); }

inline void DEFAULT_MODE(void) { printf("\033[0m"); }

char *unitest_assert(void *object, bool (*unary_predicate)(void *),
                     const char *failed_prompt_formatter, ...);

typedef char *(*AssertionFuncType)(void *object,
                                   bool (*unary_predicate)(void *obj),
                                   const char *failed_prompt_formatter, ...);

typedef char *(*AssertionWrapper)(void);

typedef struct {
    const char **func_name;   // heap-allocated, but not the elements
    const char **err_msg;     // heap-allocated, including the elements
    const char **stdout_msg;  // heap-allocated, including the elements
    AssertionWrapper *wrappers;
    usize size;
    usize cap;
} TestMap;

void testmap_new_with_capacity(TestMap *map, usize capacity);

inline void testmap_new(TestMap *map) { testmap_new_with_capacity(map, 1); }

void testmap_push(TestMap *map, const char *f_name, AssertionWrapper wrapper);

void testmap_run_sequentially(TestMap *map);

usize *testmap_run_parallelism(TestMap *map);

void testmap_dump_stdout(TestMap *map, const char *to);

void testmap_free(TestMap *map);

#endif