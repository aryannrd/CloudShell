cmake_minimum_required(VERSION 4.2)
project(mock_shell C)

set(CMAKE_C_STANDARD 11)

add_executable(mock_shell src/main.c
        include/myshell.h
        src/parser.c
        src/executor.c
        src/builtins.c
        src/pipes.c
        src/redirect.c
        src/jobs.c
        src/signals.c
        src/util.c)
