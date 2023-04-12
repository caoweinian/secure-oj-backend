#include "./cstr.h"

#include <ctype.h>
#include <stdlib.h>
#include <string.h>

#include "./error.h"

#define CAN_WRITE(ptr_cstr) (ptr_cstr->cap - ptr_cstr->end - 1)
#define CAPACITY(ptr_cstr) (ptr_cstr->cap - ptr_cstr->cstr)
#define SIZE(ptr_cstr) (ptr_cstr->end - ptr_cstr->cstr)

#define INIT_MALLOC_SIZE 128

extern usize cstr_size(CString *cstr);

extern usize cstr_memsize(CString *cstr);

extern void cstr_write_ch_unchecked(CString *cstr, char c);

extern void cstr_write_ch(CString *cstr, char c);

extern void cstr_write_unchecked(CString *cstr, const char *s);

extern void cstr_write(CString *cstr, const char *s);

extern void cstr_pop_n(CString *cstr, usize n);

extern char cstr_pop(CString *cstr);

extern char cstr_last(CString *cstr);

extern char cstr_last_unchecked(CString *cstr);

extern bool cstr_is_empty(CString *cstr);

extern void cstr_free(CString *cstr);

inline bool cstr_is_empty(CString *cstr);

inline void cstr_clear(CString *cstr);

static inline usize max(usize lhs, usize rhs) {
    return (lhs > rhs ? lhs : rhs);
}

static inline void cstr_inner_alloc(CString *cstr, usize sz) {
    cstr->cstr = (char *)malloc(sz);
    if (cstr->cstr == NULL) {
        fatal_alloc();
    }
    cstr->cstr[0] = '\0';
    cstr->end = cstr->cstr;
    cstr->cap = cstr->cstr + sz;
}

void cstr_new_empty_with_capacity(CString *cstr, usize cap) {
    // cap includes trailing '\0', we therefore need (cap + 1) byte(s).
    cstr_inner_alloc(cstr, cap + 1);
}

void cstr_new_empty(CString *cstr) { cstr_inner_alloc(cstr, 1); }

void cstr_ensure_capacity(CString *cstr, usize cap) {
    usize current_ch_can_hold = CAPACITY(cstr) - 1;
    if (current_ch_can_hold < cap) {
        usize max_val = max(cap + 1, 2 * CAPACITY(cstr));
        usize diff1 = SIZE(cstr);
        char *new_buf = (char *)realloc(cstr->cstr, max_val);
        if (new_buf == NULL) {
            fatal_alloc();
        }
        cstr->cstr = new_buf;
        cstr->end = new_buf + diff1;
        cstr->cap = new_buf + max_val;
    }
}

void cstr_ensure_remains_capacity(CString *cstr, usize cap) {
    usize space = CAN_WRITE(cstr);
    if (space < cap) {
        usize diff1 = SIZE(cstr);
        usize diff2 = CAPACITY(cstr);
        usize max_val = max(2 * diff2, diff1 + cap + 1);
        char *new_buf = (char *)realloc(cstr->cstr, max_val);
        if (new_buf == NULL) {
            fatal_alloc();
        }
        cstr->cstr = new_buf;
        cstr->end = new_buf + diff1;
        cstr->cap = new_buf + max_val;
    }
}

void cstr_new_with_str(CString *cstr, const char *s, isize sz_hint) {
    usize len = (sz_hint < 0 ? strlen(s) : (usize)sz_hint);
    cstr_new_empty_with_capacity(cstr, len);
    // cstr_ensure_capacity(cstr, len);
    cstr_write_unchecked(cstr, s);
}

usize cstr_shrink_to_fit(CString *cstr) {
    usize sz = SIZE(cstr);
    cstr->cstr = realloc(cstr->cstr, sz + 1);  // never fails
    cstr->end = cstr->cstr + sz;
    cstr->cap = cstr->cstr + sz + 1;
    return sz;
}

usize cstr_write_n_unchecked(CString *cstr, const char *s, usize n) {
    usize wrote = 0;
    while (*s != '\0' && wrote < n) {
        wrote++;
        *cstr->end++ = *s++;
    }
    *cstr->end = '\0';
    return wrote;
}

usize cstr_write_n(CString *cstr, const char *s, usize n) {
    cstr_ensure_remains_capacity(cstr, n);
    return cstr_write_n_unchecked(cstr, s, n);
}

void cstr_resize_to(CString *cstr, usize len, char fill) {
    char *flag_pos = cstr->cstr + len;
    if (flag_pos < cstr->end) {
        cstr->end = flag_pos;
        *cstr->end = '\0';
    } else if (flag_pos > cstr->end) {
        if (flag_pos >= cstr->cap) {
            cstr_ensure_capacity(cstr, len + 1);
            flag_pos = cstr->cstr + len;
        }
        while (cstr->end != flag_pos) {
            *cstr->end++ = fill;
        }
        *cstr->end = '\0';
    }
}

// #ifdef SIMPLE_CSTRING_FORMAT

void cstr_write_fmt(CString *cstr, const char *fmt, ...) {
    va_list vp;
    va_start(vp, fmt);
    cstr_write_vfmt(cstr, fmt, vp);
    va_end(vp);
}

void cstr_write_vfmt(CString *cstr, const char *fmt, va_list vp) {
    char *temp = NULL;
    usize sz = vformat(&temp, fmt, vp);
    cstr_ensure_remains_capacity(cstr, sz);
    cstr_write_unchecked(cstr, temp);
    free(temp);
}

// #endif

static inline char as_printable(char ch) { return isprint(ch) ? ch : '?'; }

usize vformat(char **to, const char *fmt, va_list marker) {
    static char buf[32];
    usize float_buf_len = 32;
    char *float_buf = (char *)malloc(sizeof(char) * 32);
    if (float_buf == NULL) {
        fatal_alloc();
    }
    CString cstr;
    cstr_new_empty_with_capacity(&cstr, INIT_MALLOC_SIZE);
    char temp = '\0';
    usize cursor = 0;
    int bool_val = 0;
    const char *s = NULL;
    f64 f64_val = 0.0;
    for (; fmt[cursor] != '\0'; ++cursor) {
        if (fmt[cursor] == '{') {
            ++cursor;
            if (fmt[cursor] == '{') {  // escape ("...{{..." -> "...{...")
                cstr_write_ch(&cstr, '{');
            } else {  // "...{..."
                temp = fmt[cursor++];
                switch (temp) {
                    case 'b':
                        bool_val = va_arg(marker, int);
                        cstr_ensure_capacity(&cstr, 5);
                        cstr_write_unchecked(&cstr,
                                             bool_val != 0 ? "true" : "false");
                        break;
                    case 's':
                        s = va_arg(marker, const char *);
                        cstr_write(&cstr, s);
                        break;
                    case 'i':
                        temp = fmt[cursor++];
                        if (temp == '4') {
                            sprintf(buf, "%d", va_arg(marker, i32));
                        } else if (temp == '8') {
                            sprintf(buf, "%ld", va_arg(marker, i64));
                        } else if (temp == '1') {
                            sprintf(buf, "%hhd", (i8)va_arg(marker, i32));
                        } else if (temp == '2') {
                            sprintf(buf, "%hd", (i16)va_arg(marker, i32));
                        }
                        cstr_write(&cstr, buf);
                        break;
                    case 'u':
                        temp = fmt[cursor++];
                        if (temp == '8') {
                            sprintf(buf, "%lu", va_arg(marker, u64));
                        } else if (temp == '4') {
                            sprintf(buf, "%u", va_arg(marker, u32));
                        } else if (temp == '1') {
                            sprintf(buf, "%hhu", (u8)va_arg(marker, i32));
                        } else if (temp == '2') {
                            sprintf(buf, "%hu", (u16)va_arg(marker, i32));
                        }
                        cstr_write(&cstr, buf);
                        break;
                    case 'c':
                        cstr_ensure_capacity(&cstr, 1);
                        cstr_write_ch_unchecked(&cstr,
                                                (char)va_arg(marker, i32));
                        break;
                    case 'f':
                        temp = fmt[cursor++];
                        f64_val = va_arg(marker, f64);
                        if (temp == '8') {
                            while (snprintf(float_buf, float_buf_len, "%lf",
                                            f64_val) >= float_buf_len) {
                                float_buf_len *= 2;
                                if ((float_buf = (char *)realloc(
                                         float_buf, float_buf_len)) == NULL) {
                                    fatal_alloc();
                                }
                            }
                        } else if (temp == '4') {
                            while (snprintf(float_buf, float_buf_len, "%f",
                                            (f32)f64_val) >= float_buf_len) {
                                float_buf_len *= 2;
                                if ((float_buf = (char *)realloc(
                                         float_buf, float_buf_len)) == NULL) {
                                    fatal_alloc();
                                }
                            }
                        }
                        cstr_write(&cstr, float_buf);
                        break;
                    default:
                        break;
                }
            }
        } else {
            cstr_write_ch(&cstr, fmt[cursor]);
        }
    }
    usize len = cstr_shrink_to_fit(&cstr);
    *to = cstr.cstr;
    free(float_buf);
    return len;
}

usize format(char **to, const char *fmt, ...) {
    va_list vp;
    va_start(vp, fmt);
    usize len = vformat(to, fmt, vp);
    va_end(vp);
    return len;
}

void sformat(CString *cstr, const char *fmt, ...) {
    va_list vp;
    va_start(vp, fmt);
    vsformat(cstr, fmt, vp);
    va_end(vp);
}

void vsformat(CString *cstr, const char *fmt, va_list vp) {
    char *handle = NULL;
    usize len = vformat(&handle, fmt, vp);
    cstr->cstr = handle;
    cstr->end = handle + len;
    cstr->cap = handle + len + 1;
}
