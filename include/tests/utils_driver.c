#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "../utils/prelude.h"

inline void switch_to(const char *color_mode) { printf("%s", color_mode); }

inline bool lexord_eq(const char *lhs, const char *rhs) {
    return strcmp(lhs, rhs) == 0;
}

inline bool lexord_ne(const char *lhs, const char *rhs) {
    return !lexord_eq(lhs, rhs);
}

extern void switch_to(const char *);
extern bool lexord_eq(const char *, const char *);
extern bool lexord_ne(const char *, const char *);

void assertions_cstr_new_empty_with_capacity() {
    CString s;
    cstr_new_empty_with_capacity(&s, 0);
    assert(*s.cstr == '\0');
    assert(s.cstr == s.end);
    assert(s.cap - s.cstr == 1);
    cstr_free(&s);

    cstr_new_empty_with_capacity(&s, 10);
    cstr_write(&s, "abcdef");
    assert(lexord_eq(s.cstr, "abcdef"));
    cstr_write(&s, "x");
    assert(lexord_eq(s.cstr, "abcdefx"));
    cstr_free(&s);

    cstr_new_empty_with_capacity(&s, 2);
    assert(cstr_memsize(&s) == 3);
    cstr_write(&s, "123456");
    assert(lexord_eq(s.cstr, "123456"));
    assert(cstr_size(&s) == 6);
    cstr_free(&s);
}

void assertions_cstr_new_empty() {
    CString s;
    cstr_new_empty(&s);
    assert(s.cstr == s.end);
    assert(s.cap - s.cstr == 1);
    assert(*s.cstr == '\0');
    cstr_free(&s);
}

void assertions_cstr_new_with_str() {
    CString s;
    char buf[32] = {0};
    for (usize i = 0; i != 20; ++i) {
        buf[i] = 'A';
    }
    cstr_new_with_str(&s, buf, -1);
    assert(cstr_size(&s) == 20);
    assert(lexord_eq(s.cstr, "AAAAAAAAAAAAAAAAAAAA"));
    cstr_write(&s, "BBB");
    assert(lexord_eq(s.cstr, "AAAAAAAAAAAAAAAAAAAABBB"));
    cstr_free(&s);
}

void assertions_cstr_ensure_capacity() {
    CString s;
    cstr_new_empty(&s);
    cstr_ensure_capacity(&s, 15);
    char *const b = s.cstr, *const e = s.end, *const c = s.cap;
    assert(s.cap - s.cstr >= 16);
    for (usize i = 0; i != 16; ++i) {
        cstr_ensure_capacity(&s, i);
    }
    assert(lexord_eq(b, s.cstr));
    assert(e == s.end);
    assert(c == s.cap);
    usize diff1 = s.end - s.cstr;
    for (usize i = 16; s.cstr == b; ++i) {
        cstr_ensure_capacity(&s, i);
    }
    assert(b != s.cstr);
    assert(e != s.end);
    assert(s.cstr + diff1 == s.end);
    assert(s.cap - s.cstr > c - b);
    cstr_free(&s);
}

void assertions_cstr_ensure_remains_capacity() {
    char buf[1000] = {0};
    for (usize i = 0; i <= 32; ++i) {
        for (usize j = 1; j <= 100; ++j) {
            CString s;
            if (i > 0) {
                buf[i - 1] = 'a';
            }
            cstr_new_with_str(&s, buf, i);
            cstr_ensure_remains_capacity(&s, j);
            const char *const cur_beg = s.cstr;
            for (usize k = 0; k != j; ++k) {
                cstr_write_ch(&s, 'x');
            }
            assert(cur_beg == s.cstr);
            assert(s.end < s.cap);
            cstr_free(&s);
        }
    }
}

void assertions_shrink_to_fit() {
    CString s;
    cstr_new_empty_with_capacity(&s, 200);
    usize sz = cstr_shrink_to_fit(&s);
    assert(sz == 0);
    assert(s.end == s.cstr);
    assert(s.cap - s.cstr == 1);
    assert(*s.cstr == '\0');
    cstr_free(&s);
    cstr_new_with_str(&s, "xyz123abc", 9);
    cstr_ensure_capacity(&s, 30);
    assert(cstr_memsize(&s) >= 31);
    cstr_shrink_to_fit(&s);
    assert(cstr_size(&s) == 9);
    assert(cstr_memsize(&s) == 10);
    cstr_free(&s);
}

void assertions_cstr_write() {
    char buf[1024] = {0};
    const char *s[] = {"abc", "1234", "xy"};
    for (usize i = 0; i != 50; ++i) {
        for (usize j = 0; j != 3; ++j) {
            strcat(buf, s[j]);
        }
    }
    CString cs;
    cstr_new_empty(&cs);
    for (usize i = 0; i != 50; ++i) {
        for (usize j = 0; j != 3; ++j) {
            cstr_write(&cs, s[j]);
        }
    }
    assert(lexord_eq(buf, cs.cstr));
    cstr_free(&cs);
}

void assertions_cstr_pop_n() {
    CString s;
    cstr_new_with_str(&s, "1234567890", 10);
    assert(cstr_size(&s) == 10);
    assert(cstr_memsize(&s) > 10);
    const char *const b = s.cstr;
    const char *const e = s.end;
    const char *const c = s.cap;
    cstr_pop_n(&s, 3);
    assert(lexord_eq(s.cstr, "1234567"));
    assert(s.cstr == b);
    assert(s.end + 3 == e);
    assert(s.cap == c);
    cstr_pop_n(&s, 5);
    assert(lexord_eq(s.cstr, "12"));
    assert(s.cstr == b);
    assert(cstr_size(&s) == 2);
    assert(cstr_memsize(&s) == c - b);
    cstr_free(&s);
}

void assertions_cstr_pop() {
    CString s;
    cstr_new_empty(&s);
    for (char ch = 'a'; (int)ch <= 'z'; ++ch) {
        cstr_write_ch(&s, ch);
    }
    // now s -> "abcdefghijklmnopqrstuvwxyz"
    for (char ch = 'z'; ch >= 'a'; --ch) {
        char p = cstr_pop(&s);
        assert(p == ch);
        assert(cstr_size(&s) == ch - 'a');
    }
    cstr_free(&s);
}

void assertions_cstr_resize_to() {
    CString s;
    const char *orig = "abcdabcdabcdabcd";
    cstr_new_with_str(&s, orig, -1);  // length -> 16
    const char *const b = s.cstr;
    const char *const e = s.end;
    const char *const c = s.cap;
    cstr_resize_to(&s, 16, '?');
    assert(lexord_eq(s.cstr, orig));
    cstr_resize_to(&s, 4, '!');
    assert(lexord_eq(s.cstr, "abcd"));
    assert(cstr_size(&s) == 4);
    assert(b == s.cstr);
    assert(e - b > s.end - s.cstr);
    assert(c == s.cap);
    cstr_resize_to(&s, 9, ':');
    assert(lexord_eq(s.cstr, "abcd:::::"));
    assert(cstr_size(&s) == 9);
    cstr_free(&s);
}

void assertions_cstr_write_fmt() {
    CString s;
    cstr_new_empty(&s);
    cstr_write_fmt(&s, "qq{b}{b}{{}}{i1}??{u8}", false, true, -128, -1);
    assert(lexord_eq(s.cstr, "qqfalsetrue{}}-128??4294967295"));
    cstr_write_fmt(&s, "{{{{}}}}");
    assert(lexord_eq(s.cstr, "qqfalsetrue{}}-128??4294967295{{}}}}"));
    cstr_free(&s);
}

void assertions_timing() {
    TimeDiffType t1 = timing_start();
    sleep(1);
    MicroSecType s = timing_stop(t1);
    GREEN_MODE();
    printf("info");
    DEFAULT_MODE();
    printf(": the value that should be >= 1000000 is %lu.\n", s);
    assert(s >= 1000000);
}

void assertions_format() {
    char *value = NULL;
    usize len = 0;
    len = format(&value, "{b}", 1);
    assert(lexord_eq(value, "true"));
    assert(len == 4);
    free(value);
    len = format(&value, "{b}", false);
    assert(lexord_eq(value, "false"));
    assert(len == 5);
    free(value);
    len = format(&value, "{i1}", 120);
    assert(lexord_eq(value, "120"));
    assert(len == 3);
    free(value);
    len = format(&value, "{i1}", 0x100 + 1);
    assert(lexord_eq(value, "1"));
    assert(len == 1);
    free(value);
    len = format(&value, "{i1}", 128);
    assert(lexord_eq(value, "-128"));
    assert(len == 4);
    free(value);
    char num[50];
    for (usize i = 0; i != 500; ++i) {
        sprintf(num, "%hhu", (u8)i);
        len = format(&value, "{u1}", i);
        assert(lexord_eq(value, num));
        free(value);
    }
    len = format(&value, "{f4}+{f8}={f8}({b}); {c}-{c}={c}({b}){s}", 1.0, 1.5,
                 2.5, true, 'c', 'b', 'a', false, "some string");
    assert(lexord_eq(
        value, "1.000000+1.500000=2.500000(true); c-b=a(false)some string"));
    free(value);
    len = format(&value, "{{ab");
    assert(lexord_eq(value, "{ab"));
    free(value);
}

int main() {
    assertions_cstr_new_empty_with_capacity();
    assertions_cstr_new_empty();
    assertions_cstr_new_with_str();
    assertions_cstr_ensure_capacity();
    assertions_cstr_ensure_remains_capacity();
    assertions_shrink_to_fit();
    assertions_cstr_write();
    assertions_cstr_pop_n();
    assertions_cstr_pop();
    assertions_cstr_write_fmt();
    assertions_timing();
    assertions_format();

    GREEN_MODE();
    puts("/include/utils/* ALL PASSED.");
    DEFAULT_MODE();
}