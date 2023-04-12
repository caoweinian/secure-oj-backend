#include <unistd.h>

#include "../utils/prelude.h"
#include "./units/prelude.h"
int main(void) {
    TestMap tm;
    testmap_new_with_capacity(&tm, 10);
    testmap_push(&tm, "space_exceeded1", space_exceeded1);
    testmap_push(&tm, "presentation_error1", presentation_error1);
    testmap_push(&tm, "accepted1", accepted1);
    testmap_push(&tm, "wrong_answer1", wrong_answer1);
    testmap_push(&tm, "invalid_ops1", invalid_ops1);
    testmap_push(&tm, "space_exceeded2", space_exceeded2);
    testmap_push(&tm, "time_exceeded1", time_exceeded1);
    testmap_run_sequentially(&tm);
    testmap_dump_stdout(&tm, NULL);
    testmap_free(&tm);
    remove_compilation_output();
}