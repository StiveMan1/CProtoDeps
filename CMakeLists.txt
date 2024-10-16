cmake_minimum_required(VERSION 3.29)
project(CProtoDeps C)

set(CMAKE_C_STANDARD 11)

include_directories(src)

add_executable(CProtoDeps main.c
    src/cpd.c
    src/cpd.h
        main.c)
