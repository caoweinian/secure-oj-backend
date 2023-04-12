#ifndef OJ_TESTS_UNITS_PRESENTATION_ERROR_H
#define OJ_TESTS_UNITS_PRESENTATION_ERROR_H

#include "../../judge.h"
#include "../../utils/prelude.h"
#include "./tools.h"

char *presentation_error1(void) {
    char *wkdir_path = workdir_concat("2");
    char *case_path = case_concat("1");
    JudgeArguments jarg = {
        .limits =
            {
                .case_count = 4,
                .max_code_bytes = 65536,
                .max_time_limit_milli = 100,
                .max_space_limit_mb = 100,
            },
        .language = Cplusplus,
        .io =
            {
                .filename = "main.cpp",
                .indirect_workdir_path = wkdir_path,
                .indirect_cases_path = case_path,
            },
        .any = NULL,
    };
    JudgeResult *result = judge(&jarg);
    free(wkdir_path);
    free(case_path);
    SingleCaseResult scr_arr[4], single = SinglePresentationError;
    blkmemcpy(scr_arr, (void *)(&single), sizeof(SingleCaseResult), 4);
    JudgeResult should_be = {
        .code = InternalStateOk,
        .prompt_message = NULL,
        .final = AllNotAccepted,
        .single_length = 4,
        .single_results = scr_arr,
        .any = NULL,
    };

    char *result_stdout = JR_stringify(result);
    char *should_be_stdout = JR_stringify(&should_be);
    printf("%s\n%s", result_stdout, should_be_stdout);
    free(result_stdout);
    free(should_be_stdout);

    char *s =
        JR_difference(result, &should_be, JRDIFF_COMPARE_EXCEPT_ANY, NULL);
    char *err_s = func_proxy(result, (s == NULL), "{s}", str_stringify(s, ""));
    free(s);
    free_judge_result(result);
    return err_s;
}

#endif