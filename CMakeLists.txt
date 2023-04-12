cmake_minimum_required(VERSION 3.16)
project(judge_lib C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_FLAGS_DEBUG "-O0")
set(CMAKE_C_FLAGS_RELEASE "-O2")

SET(CMAKE_BUILD_TYPE "Debug")

set(UTILS_DIR ${PROJECT_SOURCE_DIR}/include/utils)
set(TESTS_DIR ${PROJECT_SOURCE_DIR}/include/tests)
set(LIB_DIR ${PROJECT_SOURCE_DIR}/include)

include_directories(${UTILS_DIR})

link_directories(${PROJECT_SOURCE_DIR}/target)

add_library(utils SHARED ${UTILS_DIR}/cstr.c ${UTILS_DIR}/error.c
    ${UTILS_DIR}/timing.c ${UTILS_DIR}/unitest.c)
link_libraries(utils)

add_library(oj SHARED ${LIB_DIR}/judge.c ${LIB_DIR}/futil.c ${LIB_DIR}/usglim.c
    ${LIB_DIR}/monitor.c)
link_libraries(oj)

add_executable(utilstest ${TESTS_DIR}/utils_driver.c)
target_link_libraries(utilstest utils)

add_executable(ojtest ${TESTS_DIR}/oj_driver.c ${TESTS_DIR}/units/tools.c)
target_link_libraries(ojtest utils oj)