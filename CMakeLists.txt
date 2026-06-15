cmake_minimum_required(VERSION 4.2)
project(mock_shell C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fsanitize=address -g")
add_executable(mock_shell src/main.c
        include/myshell.h
        src/parser.c
        src/executor.c
        src/builtins.c
        src/pipes.c
        src/redirect.c
        src/signals.c
        src/util.c
        src/telemetry.c
)
list(APPEND CMAKE_PREFIX_PATH "/opt/homebrew")
find_package(CURL REQUIRED)
target_link_libraries(mock_shell CURL::libcurl)
find_package(cjson REQUIRED)
target_link_libraries(mock_shell
        CURL::libcurl
        cjson
)
