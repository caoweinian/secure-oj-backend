## A dynamic library on Linux for glibc to ensure secure judging

I am very pleased to share my college graduation project to anyone who shows great interest in OnlineJudge(especially the topic of security), and I will be thankful for your issues and suggestions.

It is licensed under the MIT License.

## Introduction

Secure-oj-backend(SOB, similarly hereinafter) generates a set of libraries and binaries, of which the binaries are built only for unit & integrated tests. We does not have TravisCI as it is managed on gitee instead of github. As a compromise, every time before the local commits are pushed, we make sure both the unit & integrated tests pass. The judging library will work when copied to /usr/lib, and the root permission may be required at that time.

- SOB is small. The final shared library can be built by headers/sources in "/include/\*.{h+c}" which currently have no more than 1,000 lines of C code.

- SOB is fast. It does not depend on external libraries. All the dependencies we need are C standard library(C11) and SUSv4 API. In addition, I/O are all employed via low-level abstractions such as file descriptor, read() and write() instead of FILE*, fscanf(), fprintf() or any akin functions in &lt;stdio.h&gt;.
  
- SOB is easy-to-use. It only exposes two functions:
    ```C
    JudgeResult *judge(JudgeArguments *args);
    void free_judge_result(JudgeResult *result);
    ```
    The <u>args</u> is prepared often by the backend codes of server, and <u>judge()</u> returns a pointer pointed to a heap-allocated structure <u>JudgeResult</u> symbolizing the judging result. You must invoke <u>free_judge_result()</u> to release the previous-allocated memory afterwards if it is <b>NOT <i>NULL</i></b>.

## Versioning(version 0.1.0, 2021-04-25)

The versioning of SOB strictly abides by the [Semantic Versioning 2.0.0](https://semver.org/). In the foreseeable future, the PATCH version will change continually.

## Compatibility

SOB is designed only on Linux of version 2.6.30 or latter. It does compile and work correctly in some former vers, though, we do not give a certain promise. 

## Installation

SOB uses [CMake](https://cmake.org/) as its dependency. To generate the library, use the following command in the top-level directory:
```bash
./mn.sh build
```
On successful build you will find all that are needed in /target/*, in which the "liboj.so" includes <u>judge()</u> and <u>free_judge_result()</u> mentioned before. The binary "utilstest" tests the dependencies of my test framework, and the binary "ojtest"(requires root permission) tests the SOB itself.

To run them both all in one command, just do as follows:
```bash
./mn.sh t
```