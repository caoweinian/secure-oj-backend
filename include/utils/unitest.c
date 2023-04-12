#include "./unitest.h"

#include <errno.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "./cstr.h"

#define FREAD_STACK_BUF 4096

extern void GREEN_MODE(void);
extern void RED_MODE(void);
extern void DEFAULT_MODE(void);

char *unitest_assert(void *object, bool (*unary_predicate)(void *),
                     const char *failed_prompt_formatter, ...) {
    bool result = unary_predicate(object);
    char *err_msg = NULL;
    if (!result) {
        va_list vp;
        va_start(vp, failed_prompt_formatter);
        vformat(&err_msg, failed_prompt_formatter, vp);
        va_end(vp);
    }
    return err_msg;
}

void testmap_new_with_capacity(TestMap *map, usize capacity) {
    const char **f = NULL;
    const char **e = NULL;
    const char **s = NULL;
    AssertionWrapper *w = NULL;
    if ((f = (const char **)malloc(capacity * sizeof(const char *))) == NULL ||
        (e = (const char **)malloc(capacity * sizeof(const char *))) == NULL ||
        (s = (const char **)malloc(capacity * sizeof(const char *))) == NULL ||
        (w = (AssertionWrapper *)malloc(capacity * sizeof(AssertionWrapper))) ==
            NULL) {
        fatal_alloc();
    }
    for (usize i = 0; i != capacity; ++i) {
        f[i] = NULL;
        e[i] = NULL;
        s[i] = NULL;
        w[i] = NULL;
    }
    map->size = 0;
    map->cap = capacity;
    map->func_name = f;
    map->err_msg = e;
    map->stdout_msg = s;
    map->wrappers = w;
}

extern void testmap_new(TestMap *map);

static inline void testmap_ensure_capacity(TestMap *map) {
    if (map->size == map->cap) {
        usize twice = 2 * map->size;
        const char **f = (const char **)realloc(map->func_name,
                                                twice * sizeof(const char *));
        if (f == NULL) {
            fatal_alloc();
        }
        const char **e =
            (const char **)realloc(map->err_msg, twice * sizeof(const char *));
        if (e == NULL) {
            fatal_alloc();
        }
        const char **s = (const char **)realloc(map->stdout_msg,
                                                twice * sizeof(const char *));
        if (s == NULL) {
            fatal_alloc();
        }
        AssertionWrapper *w = (AssertionWrapper *)realloc(
            map->wrappers, twice * sizeof(AssertionFuncType));
        if (w == NULL) {
            fatal_alloc();
        }
        for (usize i = map->size; i != twice; ++i) {
            f[i] = NULL;
            e[i] = NULL;
            s[i] = NULL;
            w[i] = NULL;
        }
        map->cap = twice;
        map->func_name = f;
        map->err_msg = e;
        map->stdout_msg = s;
        map->wrappers = w;
    }
}

void testmap_push(TestMap *map, const char *f_name, AssertionWrapper wrapper) {
    testmap_ensure_capacity(map);
    map->func_name[map->size] = f_name;
    map->wrappers[map->size] = wrapper;
    ++map->size;
}

void testmap_run_sequentially(TestMap *map) {
    printf("running %lu test%c\n", map->size, "s"[map->size <= 1]);
    usize passed_count = 0;
    MicroSecType total_time = 0;
    FILE *const origin_stdout = stdout;
    char stdout_buf[FREAD_STACK_BUF];
    for (usize i = 0; i != map->size; ++i) {
        // system("ls /proc/self/fd -alF");
        printf("test %s ... ", map->func_name[i]);
        fflush(stdout);
        stdout = tmpfile();
        TimeDiffType d = timing_start();

        const char *result = map->wrappers[i]();
        total_time += timing_stop(d);
        if (fseek(stdout, 0, SEEK_SET) == -1) {
            fatal("fseek(%s)", strerror(errno));
        }
        CString cstr;
        cstr_new_empty_with_capacity(&cstr, FREAD_STACK_BUF);
        usize read_bytes = 0;
        while ((read_bytes = fread(stdout_buf, 1, FREAD_STACK_BUF, stdout)) ==
               FREAD_STACK_BUF) {
            cstr_write_n(&cstr, stdout_buf, read_bytes);
        }
        if (read_bytes != 0) {
            cstr_write_n(&cstr, stdout_buf, read_bytes);
        } else if (ferror(stdout)) {
            fatal("ferror");
        }
        fclose(stdout);
        stdout = origin_stdout;
        map->stdout_msg[i] = cstr.cstr;
        if (result == NULL) {
            passed_count++;
            printf("%sok\n", GREEN_STR);
        } else {
            printf("%sFAILED\n", RED_STR);
            map->err_msg[i] = result;
        }
        DEFAULT_MODE();
    }
    putchar('\n');
    if (passed_count == map->size) {
        printf(
            "test result: %sok%s. %lu passed; 0 failed; finished in %.2lfs\n",
            GREEN_STR, DEFAULT_STR, passed_count, (f64)total_time / 1000000);
    } else {
        printf("failures:\n\n");
        for (usize i = 0; i != map->size; ++i) {
            if (map->err_msg[i] != NULL) {
                printf("---- %s error detail ----\n%s\n\n", map->func_name[i],
                       map->err_msg[i]);
            }
        }
        printf("\nfailures:\n");
        for (usize i = 0; i != map->size; ++i) {
            if (map->err_msg[i] != NULL) {
                printf("\t%s\n", map->func_name[i]);
            }
        }
        printf(
            "\ntest result: %sFAILED%s. %lu passed; %lu failed; finished in "
            "%.2lfs\n",
            RED_STR, DEFAULT_STR, passed_count, map->size - passed_count,
            (f64)total_time / 1000000);
    }
}

usize *testmap_run_parallelism(TestMap *map) {
    fatal(
        "function \"testmap_run_parallelism\" not implemented yet.\n In fact, "
        "to run the task in parallel requires"
        " pthread, which is rather difficult to handle correctly. This is to "
        "be considered after when the project"
        " be almost completed.\n");
    return 0;
}

void testmap_dump_stdout(TestMap *map, const char *to) {
    FILE *out = fopen((to == NULL ? "./LOG" : to), "w");
    if (out == NULL) {
        fatal("%s(in dump_stdout)\n", strerror(errno));
    }
    for (usize i = 0; i != map->size; ++i) {
        fprintf(out, "---- %s stdout ----\n%s\n", map->func_name[i],
                (map->stdout_msg[i] == NULL ? "(Nil)" : map->stdout_msg[i]));
    }
    fprintf(out, "---- END ----");
    fclose(out);
}

void testmap_free(TestMap *map) {
    for (usize i = 0; i != map->size; ++i) {
        free((void *)map->err_msg[i]);
        free((void *)map->stdout_msg[i]);
    }
    free(map->func_name);
    free(map->err_msg);
    free(map->stdout_msg);
    free(map->wrappers);
}