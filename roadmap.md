# Development Log
1. \[development\] Finish a demo using C++ and ensure the feasibility/tech details of total pass
    - since January 20th
    - done on February 22th
2. \[development\] Re-implement it by C, let it compile and work 
    - since April 11th
    - done on April 20th
3. \[bug-fix\] Close "source_code_relative_path_fd" in "./judge.c" under any execution branch 
    - issued on April 21th
    - done on April 21th
4. \[feature\] Provide a flexible way to compare two JudgeResult
    - issued on April 21th
    - done on April 21th
5. \[optimization\] Implement a FdBuf to cache Disk&SSD reads to compare two files faster
    - issued on April 21th
    - done on April 21th
6. \[feature\] Add multi-thread support using pthread_t for test framework
    - issued on April 21th
7. \[test\] Add more tests to cover more cases, especially those that show robustness and security.
    - issued on April 21th
8. \[bug-fix\] Admit first system call of SYS_execve, but refuse its subsequent possible appearances
    - issued on April 21th
    - done on April 21th
9. \[bug-fix, lifetime\] Track memory blocks that are not freed normally
    - issued on April 21th
10. \[refactor\] Separate utility function in "./judge.c" to another file, leaving the core implementations "judge" and "free_judge_result" in it
    - issued on April 21th
    - done on April 21th
11. \[feature\] Add capacity field and corresponding functions for TestMap
    - issued on April 21th
    - already done
12. \[doc\] Add Rust-like documentation for every function exposed to user
    - issued on April 21th 
13. \[feature\] Add a "rusage*" field in JudgeResult
    - issued on April 21th
14. \[bug-fix\] Some presentation error belongs to wrong answer
    - issued on April 22th
15. \[feature\] Handle out-of-range error
    - issued on April 22th
