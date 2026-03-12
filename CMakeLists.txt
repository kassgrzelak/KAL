cmake_minimum_required(VERSION 4.1)
project(KAL C)

set(CMAKE_C_STANDARD 11)

add_executable(kal main.c
        include/common.h
        include/VM.h
        include/Bytecode.h
        include/memory.h
        src/memory.c
        include/Scanner.h)

target_include_directories(kal PRIVATE include)
