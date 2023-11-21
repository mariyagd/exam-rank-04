cmake_minimum_required(VERSION 3.26)
project(exam_rank_04 C)

set(CMAKE_C_STANDARD 11)

include_directories(.)

add_executable(exam_rank_04
        microshell.c
        microshell2.c
        microshell3.c
        microshell.h
        test.c)
