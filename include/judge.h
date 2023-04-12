//! All the interfaces exposed to external caller

#ifndef OJ_JUDGE_H
#define OJ_JUDGE_H

#define _GNU_SOURCE
#include <stdint.h>

/// A set including the number of test case(s) and time/space/file limit.
typedef struct {
    uint32_t case_count;
    uint32_t max_code_bytes;
    uint32_t max_time_limit_milli;
    uint32_t max_space_limit_mb;  // must be no more than 256 (256MB)
} ProblemResourceLimit;

/// The language the user uses
typedef enum {
    C = 1,
    Cplusplus = 2,
    // Rust = 3,   // not implemented yet
} Language;

/// The path at which user code and cases are located.
///
/// The `filename` field indicates the filename of user code, like "main.cpp".
///
/// The `indirect_workdir_path` field indicates the relative path from current
/// directory to `filename`.
///
/// The `indirect_cases_path` field indicates the relative path from current
/// directory to case files.
///
/// For instance, if `filename` is "main.c", `indirect_workdir_path` is
/// "judge_jail/123", so the full path of `filename` is
/// "./judge_jail/123/main.c". The same rule applies to `indirect_cases_path`.
typedef struct {
    const char *filename;
    const char *indirect_workdir_path;
    const char *indirect_cases_path;
} FilePathInfo;

/// The judging result for a single case.
///
/// WaitForJudging is a default/uninitialized value for `SingleCaseResult`.
///
/// SingleAccepted means this case produces expected output.
///
/// SingleWrongAnswer means this case produces unexpected output.
///
/// SinglePresentationError means this case produces answer with wrong format,
/// like the difference between '1 2 3' and '1 2 3 '(the redundant trailing
/// space). Also, we only accept '\n' as newline insteat of '\r' and '\r\n'.
/// This also leads to SinglePresentationError. Besides, you must accept the
/// fact that some outputs which are apparently wrong are considered as
/// SinglePresentationError, like '1 2 3\n4 5 6' and '1 2 3\r5 6 7'.
///
/// SingleSpaceLimitExceeded means the user uses space more than
/// `ProblemResourceLimit`.`max_space_limit_mb`; This does not cover all cases
/// that are de facto "memory limit exceeded". As an example, when the user
/// program allocates very very large memory(say, 2GiB) all at once, possibly it
/// is not judged as SingleSpaceLimitExceeded; Instead, the allocation may
/// return NULL.           //  -> how about the `new` in C++?
///
/// SinleTimeLimitExceeded means the user uses time more than
/// `ProblemResourceLimit`.`max_time_limit_milli`; It aims at cpu time and
/// neglects I/O time.
///
/// SingleOpterationNotPermitted means the user uses invalid system call like
/// alarm(), fork(), execl(), etc.
///
/// SingleRuntimeError means other kinds of errors.
typedef enum {
    WaitForJudging = -1,
    SingleAccepted,
    SingleWrongAnswer,
    SinglePresentationError,
    SingleSpaceLimitExceeded,
    SingleTimeLimitExceeded,
    SingleOperationNotPermitted,
    SingleRuntimeError,
} SingleCaseResult;

/// Final judgeing result.
///
/// Undefined is a default/uninitialized value for `AllCasesResult`.
///
/// AllAccepted means all the `SingleCaseResult` is SingleAccepted.
///
/// AllPartialAccepted means there exists SingleAccepted but not all.
///
/// AllNotAccepted means none of the `SingleCaseResult` is SingleAccepted.
///
/// SourceTooLong means the code bytes exceeds
/// `ProblemResourceLimit`.`max_code_bytes`.
///
/// CompilationTimeExceeded means the compiling time exceeds. It is rather
/// difficult to produce this case, but it can be made deliberately after all.
///
/// CompilationError is obvious: the user code does not compile.
typedef enum {
    Undefined = -1,
    AllAccepted,
    AllPartialAccepted,
    AllNotAccepted,
    SourceTooLong,
    CompilationTimeExceeded,
    CompilationError,
} AllCasesResult;

/// Self-explainable.
///
/// When the judging returns certain result(including CompilationTimeExceeded,
/// CompilationError and all the values in SingleCaseResult except
/// WaitForJudging), the `InternalStateCode` returns InternalStateOk.
///
/// InvalidArgument is caused when the `JudgeArguments` is invalid. It is not
/// implemented yet; User must make sure the validness of it. It is reasonable,
/// because this library is exposed to backend programmers rather than users,
/// and the former of course know how to avoid this situation.
///
/// OutOfMemory means malloc() fails.
///
/// ImNotRoot means the caller does not have root permission, which is necessary
/// for judging and monitoring.
typedef enum {
    InternalStateOk = 0,
    InvalidArgument,
    InternalError,
    OutOfMemory,
    ImNotRoot,
} InternalStateCode;

/// All arguments needed for judging.
///
/// the `any` field is extremely useful when the library makes incompatible
/// changes. To make it downword compatible, we can put ANYTHING in it without
/// memory layout modified. In this case, previous applications will run
/// normally though the library are updated. It is inspired by "any" class in
/// C++17 standard library header <any>.
typedef struct {
    ProblemResourceLimit limits;
    Language language;
    FilePathInfo io;
    void *any;
} JudgeArguments;

/// Final judging result.
///
/// When the `final` is CompilationError, the `prompt_message` is a C-style
/// string of no more than 1,000 bytes(excludes the trailing '\0') for 
/// compilation error message generated by GCC.
/// When the `final` is SourceTooLong, the `prompt_message` is a C-style string 
/// of no more than 20 bytes(excludes the trailing '\0') for a decimal number 
/// of the real bytes the user code have.
/// In other circumstances, `prompt_message` is NULL.
///
/// The `single_length` should be equal to
/// `JudgeArguments`.`limits`.`case_count` and indicates the valid length of
/// `single_results`.
///
/// The case number `k` corresponds to result `single_results`[k - 1].
///
/// The use of `any` field is similar to `JudgeArguments`.`any`.
typedef struct {
    InternalStateCode code;
    char *prompt_message;
    AllCasesResult final;
    uint32_t single_length;
    SingleCaseResult *single_results;
    void *any;
} JudgeResult;

/// Judging.
///
/// The `args` are passed by backend. It can be stack-allocated, heap-allocated
/// or in static storage. Caller must make sure the memory that `args` points to
/// is invalid before judge() returns.
///
/// The function returns NULL when there is no enough memory; or it points to a
/// valid, heap-allocated `JudgeResult` for judging result. It must be free by
/// subsequent free_judge_result() after usage.
JudgeResult *judge(const JudgeArguments *args);

/// Free memory returned by judge().
void free_judge_result(JudgeResult *result);

#endif
