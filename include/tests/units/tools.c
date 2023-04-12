#define _XOPEN_SOURCE 700
#include "./tools.h"

#include <ftw.h>
#include <stdbool.h>
#include <string.h>
#include <unistd.h>

extern const char *INDIRECT_WORKDIR_PATH_BASE;
extern const char *INDIRECT_CASES_PATH_BASE;

char *workdir_concat(const char *sub_dir) {
    char *to = NULL;
    format(&to, "{s}/{s}", INDIRECT_WORKDIR_PATH_BASE, sub_dir);
    return to;
}

char *case_concat(const char *sub_dir) {
    char *to = NULL;
    format(&to, "{s}/{s}", INDIRECT_CASES_PATH_BASE, sub_dir);
    return to;
}

const char *SCR_stringify(SingleCaseResult code) {
    switch (code) {
        case WaitForJudging:
            return "WaitForJuding";
        case SingleAccepted:
            return "SingleAccepted";
        case SingleWrongAnswer:
            return "SingleWrongAnswer";
        case SinglePresentationError:
            return "SinglePresentationError";
        case SingleSpaceLimitExceeded:
            return "SingleSpaceLimitExceeded";
        case SingleTimeLimitExceeded:
            return "SingleTimeLimitExceeded";
        case SingleOperationNotPermitted:
            return "SingleOperationNotPermitted";
        case SingleRuntimeError:
            return "SingleRuntimeError";
    }
}

const char *ACR_stringify(AllCasesResult code) {
    switch (code) {
        case Undefined:
            return "Undefined";
        case AllAccepted:
            return "AllAccepted";
        case AllPartialAccepted:
            return "AllPartialAccepted";
        case AllNotAccepted:
            return "AllNotAccepted";
        case SourceTooLong:
            return "SourceTooLong";
        case CompilationTimeExceeded:
            return "CompilationTimeExceeded";
        case CompilationError:
            return "CompilationError";
    }
}

const char *ISC_stringify(InternalStateCode code) {
    switch (code) {
        case InternalStateOk:
            return "InternalStateOk";
        case InvalidArgument:
            return "InvalidArgument";
        case InternalError:
            return "InternalError";
        case OutOfMemory:
            return "OutOfMemory";
        case ImNotRoot:
            return "ImNotRoot";
    }
}

extern const char *str_stringify(const char *s, const char *null_default);

char *JR_stringify(JudgeResult *result) {
    CString s;
    sformat(&s,
            "{{\n\tcode: {s}\n\tprompt_message: {s}\n\tfinal: {s}\n\t"
            "single_length: {u4}\n\tsingle_results: [",
            ISC_stringify(result->code),
            str_stringify(result->prompt_message, "NULL"),
            ACR_stringify(result->final), result->single_length);
    if (result->single_length != 0) {
        cstr_write_fmt(&s, "{s}", SCR_stringify(result->single_results[0]));
        for (u32 i = 1; i != result->single_length; ++i) {
            cstr_write_fmt(&s, ", {s}",
                           SCR_stringify(result->single_results[i]));
        }
    }
    cstr_write(&s, "]\n\tany: (...)\n}");
    return s.cstr;
}

char *func_proxy(void *object, bool ok, const char *fmt, ...) {
    char *to = NULL;
    if (!ok) {
        va_list vp;
        va_start(vp, fmt);
        usize sz = vformat(&to, fmt, vp);
        va_end(vp);
    }
    return to;
}

static bool ends_with(const char *src, const char *pattern) {
    usize pattern_len = strlen(pattern);
    usize src_len = strlen(src);
    return strcmp(src + (src_len - pattern_len), pattern) == 0;
}

static int nftw_callback(const char *fpath, const struct stat *statbuf,
                         int typeflag, struct FTW *ftwbuf) {
    const char *base = fpath + ftwbuf->base;
    if (typeflag == FTW_F && !ends_with(base, ".c") &&
        !ends_with(base, ".cpp") && remove(fpath) == -1) {
        fatal("cannot remove(\"%s\") in function nftw_callback\n");
    }
    return 0;
}

void remove_compilation_output(void) {
    if (nftw(INDIRECT_WORKDIR_PATH_BASE, nftw_callback, 64, 0) == -1) {
        fatal("ntfw failed\n");
    }
}

static void cstr_write_SCR_array(CString *cstr, SingleCaseResult *scr,
                                 u32 len) {
    cstr_ensure_remains_capacity(cstr, 2 + len * 30);
    cstr_write_ch(cstr, '[');
    if (len != 0) {
        cstr_write_fmt(cstr, "{s}", SCR_stringify(scr[0]));
        for (u32 i = 1; i != len; ++i) {
            cstr_write_fmt(cstr, ", {s}", SCR_stringify(scr[i]));
        }
    }
    cstr_write_ch(cstr, ']');
}

char *JR_difference(JudgeResult *result, JudgeResult *expected, i32 options,
                    JudgeResultAnyCmpFuncType func) {
    if (options == 0) {
        options = JRDIFF_COMPARE_DEFAULT;
    }
    CString s;
    cstr_new_empty_with_capacity(&s, 128);
    usize failure_no = 1;
    if ((options & JRDIFF_COMPARE_CODE) && result->code != expected->code) {
        cstr_write_fmt(&s, "failure {u8}: expected code = {s}, found {s}\n",
                       failure_no++, ISC_stringify(expected->code),
                       ISC_stringify(result->code));
    }
    if ((options & JRDIFF_COMPARE_PROMPT) &&
        !(result->prompt_message == NULL && expected->prompt_message == NULL)) {
        if (result->prompt_message == NULL) {
            cstr_write_fmt(&s,
                           "failure {u8}: expect prompt_message = \"{s}\", but "
                           "found a NULL\n",
                           failure_no++, expected->prompt_message);

        } else if (expected->prompt_message == NULL) {
            cstr_write_fmt(&s,
                           "failure {u8}: expected prompt_message with a NULL, "
                           "but found \"{s}\"\n",
                           failure_no++, result->prompt_message);
        } else {
            if (strcmp(result->prompt_message, expected->prompt_message) != 0) {
                cstr_write_fmt(&s,
                               "failure {u8}: expect prompt_message =\"{s}\", "
                               "found \"{s}\"\n",
                               failure_no++,
                               str_stringify(expected->prompt_message, "NULL"),
                               str_stringify(result->prompt_message, "NULL"));
            }
        }
    }
    if ((options & JRDIFF_COMPARE_SINGLE_LENGTH) &&
        result->single_length != expected->single_length) {
        cstr_write_fmt(
            &s, "failure {u8}: expected single_length = {u4}, found {u4}\n",
            failure_no++, expected->single_length, result->single_length);
    }
    if ((options & JRDIFF_COMPARE_SINGLE_RESULTS)) {
        bool identical = true;
        if (result->single_length != expected->single_length) {
            identical = false;
        } else {
            for (u32 i = 0; i != expected->single_length; ++i) {
                if (expected->single_results[i] != result->single_results[i]) {
                    identical = false;
                    break;
                }
            }
        }
        if (!identical) {
            cstr_write_fmt(
                &s, "failure {u8}: expected single_results = ", failure_no++);
            cstr_write_SCR_array(&s, expected->single_results,
                                 expected->single_length);
            cstr_write(&s, ", found ");
            cstr_write_SCR_array(&s, result->single_results,
                                 result->single_length);
            cstr_write_ch(&s, '\n');
        }
    }
    if (options & JRDIFF_COMPARE_ANY) {
        char *info = func(result->any, expected->any);
        if (info != NULL) {
            cstr_write_fmt(&s, "failure {u8}(any): {s}\n", info);
            free(info);
        }
    }
    if (strcmp(s.cstr, "") == 0) {
        cstr_free(&s);
        return NULL;
    }
    return s.cstr;
}

void *blkmemcpy(void *dest, const void *src, usize block_size,
                usize batch_times) {
    for (usize i = 0; i != batch_times; ++i) {
        memcpy(dest + i * block_size, src, block_size);
    }
    return dest;
}