cmake_minimum_required(VERSION 4.1)
project(KAL C)

set(CMAKE_C_STANDARD 11)

add_executable(kal main.c
        include/common.h
        include/VM.h
        include/memory.h
        src/memory.c
        include/Scanner.h
        src/Scanner.c
        include/Compiler.h
        src/Compiler.c
        include/Bytecode.h
        src/Bytecode.c
        src/VM.c
        include/instructions.h
        include/opcodes.inc
        include/mnemonicTokens.inc
)

target_include_directories(kal PRIVATE include)
