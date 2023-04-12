#ifndef OJ_CSTR_H
#define OJ_CSTR_H

#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "./nums.h"

typedef struct {
    char *cstr;
    char *end;
    char *cap;
} CString;

void cstr_new_empty_with_capacity(CString *cstr, usize cap);

void cstr_new_empty(CString *cstr);

void cstr_new_with_str(CString *cstr, const char *s, isize sz_hint);

void cstr_ensure_capacity(CString *cstr, usize cap);

void cstr_ensure_remains_capacity(CString *cstr, usize cap);

usize cstr_shrink_to_fit(CString *cstr);

inline bool cstr_is_empty(CString *cstr) { return *cstr->cstr == '\0'; }

inline char cstr_last_unchecked(CString *cstr) { return *(cstr->end - 1); }

inline char cstr_last(CString *cstr) {
    return cstr_is_empty(cstr) ? '\0' : cstr_last_unchecked(cstr);
}

inline usize cstr_memsize(CString *cstr) { return cstr->cap - cstr->cstr; }

inline usize cstr_size(CString *cstr) { return cstr->end - cstr->cstr; }

inline void cstr_write_ch_unchecked(CString *cstr, char c) {
    *cstr->end++ = c;
    *cstr->end = '\0';
}

inline void cstr_write_ch(CString *cstr, char c) {
    cstr_ensure_remains_capacity(cstr, 1);
    cstr_write_ch_unchecked(cstr, c);
}

inline void cstr_write_unchecked(CString *cstr, const char *s) {
    while (*s != '\0') {
        *cstr->end++ = *s++;
    }
    *cstr->end = '\0';
}

inline void cstr_write(CString *cstr, const char *s) {
    cstr_ensure_remains_capacity(cstr, strlen(s));
    cstr_write_unchecked(cstr, s);
}

usize cstr_write_n_unchecked(CString *cstr, const char *s, usize n);

usize cstr_write_n(CString *cstr, const char *s, usize n);

inline void cstr_pop_n(CString *cstr, usize n) {
    cstr->end -= n;
    *cstr->end = '\0';
}

inline char cstr_pop(CString *cstr) {
    char ch = *--cstr->end;
    *cstr->end = '\0';
    return ch;
}

void cstr_resize_to(CString *cstr, usize len, char fill);

inline void cstr_clear(CString *cstr) { cstr->end = cstr->cstr; }

inline void cstr_free(CString *cstr) { free(cstr->cstr); }

/// 格式化一个字符串，返回一个堆分配的空终止字符串空间
usize format(char **to, const char *fmt, ...);

usize vformat(char **to, const char *fmt, va_list vp);

void sformat(CString *cstr, const char *fmt, ...);

void vsformat(CString *cstr, const char *fmt, va_list vp);

// #ifdef SIMPLE_CSTRING_FORMAT

void cstr_write_vfmt(CString *cstr, const char *fmt, va_list vp);

void cstr_write_fmt(CString *cstr, const char *fmt, ...);

// #endif

#endif