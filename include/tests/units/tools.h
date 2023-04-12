#ifndef OJ_TESTS_UNITS_TOOLS_H
#define OJ_TESTS_UNITS_TOOLS_H

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#include "../../judge.h"
#include "../../utils/prelude.h"

#define JRDIFF_COMPARE_CODE 0x1
#define JRDIFF_COMPARE_FINAL 0x2
#define JRDIFF_COMPARE_SINGLE_LENGTH 0x4
#define JRDIFF_COMPARE_SINGLE_RESULTS 0x8
#define JRDIFF_COMPARE_DEFAULT 0xf
#define JRDIFF_COMPARE_PROMPT 0x10
#define JRDIFF_COMPARE_ANY 0x20
#define JRDIFF_COMPARE_ALL 0x3F
#define JRDIFF_COMPARE_EXCEPT_ANY 0x1f

static const char *INDIRECT_WORKDIR_PATH_BASE = "../io/submission";
static const char *INDIRECT_CASES_PATH_BASE = "../io/case";

char *workdir_concat(const char *sub_dir);

char *case_concat(const char *sub_dir);

const char *SCR_stringify(SingleCaseResult code);

const char *ACR_stringify(AllCasesResult code);

const char *ISC_stringify(InternalStateCode code);

inline const char *str_stringify(const char *s, const char *null_default) {
    return (s == NULL ? null_default : s);
}

char *JR_stringify(JudgeResult *result);

char *func_proxy(void *object, bool ok, const char *fmt, ...);

void remove_compilation_output(void);

typedef char *(*JudgeResultAnyCmpFuncType)(void *result, void *expected);

void *blkmemcpy(void *dest, const void *src, usize block_size,
                usize batch_times);

char *JR_difference(JudgeResult *result, JudgeResult *expected, i32 options,
                    JudgeResultAnyCmpFuncType func);

#endif