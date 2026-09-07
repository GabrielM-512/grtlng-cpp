cmake_minimum_required(VERSION 4.3)
project(lexer_test)

set(CMAKE_CXX_STANDARD 26)

message(WORKING_DIRECTORY)

add_executable(lexer_test
        /home/gabriel/CLionProjects/grtlng-cpp/testing/partial/lexer/lexer_test.cpp
        /home/gabriel/CLionProjects/grtlng-cpp/src/compiler/lexing.h
        /home/gabriel/CLionProjects/grtlng-cpp/src/compiler/lexing.cpp
        /home/gabriel/CLionProjects/grtlng-cpp/src/util/fileIO.h
        /home/gabriel/CLionProjects/grtlng-cpp/src/util/fileIO.cpp
)

target_compile_options(lexer_test
        PUBLIC -Wall
        PUBLIC -Wextra
        PUBLIC -Werror
)